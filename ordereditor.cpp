#include "ordereditor.h"
#include "ui_ordereditor.h" 
#include "addorderdetailwindow.h" 
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QDate>
#include <QFile>
#include <QTextStream>

OrderEditor::OrderEditor(const QString &orderId, const QSqlDatabase &db, QWidget *parent)
    : QDialog(parent), ui(new Ui::OrderEditor()), orderId(orderId), db(db), isNewOrder(orderId.isEmpty()) {
    ui->setupUi(this);
    setWindowTitle(isNewOrder ? "Add Order" : "Edit Order");

    //populate combo boxes
    populateStatusCombo();
    populateCustomerCombo();
    populatePaymentCombo();

    // Setup items model
    itemsModel = new QSqlTableModel(this, db);
    itemsModel->setTable("order_items");
    if (!isNewOrder) {
        itemsModel->setFilter(QString("order_id = '%1'").arg(orderId)); 
        // Load existing customer for edit
        QSqlQuery loadQuery(db);
        loadQuery.prepare("SELECT customer_id, status, order_date FROM orders WHERE order_id = :orderId"); 
        loadQuery.bindValue(":orderId", orderId);
        if (loadQuery.exec() && loadQuery.next()) {
            int custId = loadQuery.value("customer_id").toInt();
            int statusId = loadQuery.value("status").toInt(); 
            QDate orderDate = loadQuery.value("order_date").toDate(); 
            int custIndex = ui->customerCombo->findData(custId);
            if (custIndex >= 0) {
                ui->customerCombo->setCurrentIndex(custIndex);
            }
            int statusIndex = ui->statusCombo->findData(statusId); 
            if (statusIndex >= 0) {
                ui->statusCombo->setCurrentIndex(statusIndex);
            }
            if (ui->dateEdit) {  
                ui->dateEdit->setDate(orderDate);
            }
        } else {
            QMessageBox::warning(this, "Data Error", "Failed to load order: " + loadQuery.lastError().text());
        }
    }
    itemsModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    itemsModel->setHeaderData(0, Qt::Horizontal, "Item ID");
    itemsModel->setHeaderData(1, Qt::Horizontal, "Order ID");
    itemsModel->setHeaderData(2, Qt::Horizontal, "Detail ID"); 
    itemsModel->setHeaderData(3, Qt::Horizontal, "Quantity");
    itemsModel->setHeaderData(4, Qt::Horizontal, "Unit Price");
    itemsModel->setHeaderData(5, Qt::Horizontal, "Tax Rate (%)");
    itemsModel->setHeaderData(6, Qt::Horizontal, "Subtotal");
    itemsModel->setHeaderData(7, Qt::Horizontal, "Tax Amount");
    itemsModel->setHeaderData(8, Qt::Horizontal, "Total");
    if (!itemsModel->select()) {
        QMessageBox::warning(this, "Data Error", "Failed to load order items: " + itemsModel->lastError().text());
    }

    ui->itemsTable->setModel(itemsModel);
    ui->itemsTable->setColumnHidden(0, true);
    ui->itemsTable->setColumnHidden(1, true);
    
    ui->itemsTable->horizontalHeader()->setStretchLastSection(true);
    ui->itemsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->itemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->itemsTable->resizeColumnsToContents();

    // Connect buttons
    connect(ui->addItemButton, &QPushButton::clicked, this, &OrderEditor::addItem);
    connect(ui->editItemButton, &QPushButton::clicked, this, &OrderEditor::editItem);
    connect(ui->deleteItemButton, &QPushButton::clicked, this, &OrderEditor::deleteItem);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &OrderEditor::saveAndClose);  
    connect(ui->saveOrderButton, &QPushButton::clicked, this, &OrderEditor::saveOrderDetails);  

    if (isNewOrder) {
        // Disable item management until order is saved
        ui->addItemButton->setEnabled(false);
        ui->addItemButton->setToolTip("Save the order first");
        ui->addItemButton->setStyleSheet("background-color: #00000079; color: #000000089;");
        ui->editItemButton->setEnabled(false);
        ui->editItemButton->setStyleSheet("background-color: #00000079; color: #000000089;");
        ui->editItemButton->setToolTip("Save the order first");
        ui->deleteItemButton->setEnabled(false);
        ui->deleteItemButton->setStyleSheet("background-color: #00000079; color: #000000089;");
        ui->deleteItemButton->setToolTip("Save the order first");
        
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);  
        ui->buttonBox->button(QDialogButtonBox::Ok)->setStyleSheet("background-color: #00000079; color: #000000089;");
        ui->buttonBox->button(QDialogButtonBox::Ok)->setToolTip("Save the order header first");
        // NEW: Leave saveOrderButton enabled for new orders
    }
}

OrderEditor::~OrderEditor() {
    delete itemsModel;
    delete ui;
}

void OrderEditor::populateCustomerCombo() {
    ui->customerCombo->clear();  // Assume QComboBox *customerCombo in UI
    QSqlQuery query(db);
    query.exec("SELECT customer_id, CONCAT(first_name, ' ', last_name) AS name FROM customer ORDER BY name");
    while (query.next()) {
        ui->customerCombo->addItem(query.value("name").toString(), query.value("customer_id"));
    }
}


void OrderEditor::populateStatusCombo() {
    ui->statusCombo->clear();

    QFile file(":/order_statuses.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        
        // Fallback statuses
        ui->statusCombo->addItem("Pending", 1);
        ui->statusCombo->addItem("Shipped", 2);
        ui->statusCombo->addItem("Delivered", 3);
        return;
    }
    QTextStream in(&file);
    QStringList statuses;
    int statusId = 1;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();  
        if (!line.isEmpty()) {  
            statuses.append(line);
            ui->statusCombo->addItem(line, statusId++);  
        }
    }
    file.close();
   
    if (isNewOrder) {
        ui->statusCombo->setCurrentIndex(0);  // Pending
    }
}

void OrderEditor::populatePaymentCombo(){
    ui->paymentCombo->clear();
    ui->paymentCombo->addItem("Card");
    ui->paymentCombo->addItem("Venmo");
    ui->paymentCombo->addItem("Cash");
    if (isNewOrder) {
        ui->paymentCombo->setCurrentIndex(0);  // Default to first option
    }
}

void OrderEditor::saveOrderDetails() {
    qDebug() << "=== saveOrderDetails() START ===";

    // Validation
    if (ui->customerCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "Validation Error", "Please select a customer.");
        return;
    }
    if (ui->statusCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "Validation Error", "Please select a status.");
        return;
    }

    int custId = ui->customerCombo->currentData().toInt();
    QString statusText = ui->statusCombo->currentText();  
    QDate orderDate = ui->dateEdit->date();
    QString paymentMethod = ui->paymentCombo->currentText(); 
    


    QSqlQuery query(db);
    if (isNewOrder) {
        query.prepare("INSERT INTO orders (customer_ID, order_date, status, payment_method) "
                      "VALUES (:custId, :orderDate, :status)");
        query.bindValue(":custId", custId);
        query.bindValue(":orderDate", orderDate);
        query.bindValue(":status", statusText);
        query.bindValue(":paymentMethod", paymentMethod);
        if (!query.exec()) {
            QString err = query.lastError().text();
            qDebug() << "INSERT failed: " << err;
            QMessageBox::warning(this, "Database Error", "Failed to create order: " + err);
            return;
        }
        orderId = query.lastInsertId().toString();
        isNewOrder = false;

        // Refresh items filter
        itemsModel->setFilter(QString("order_ID = '%1'").arg(orderId));  // Schema: order_ID
      

        // Enable controls (as before)
        ui->addItemButton->setEnabled(true);
        ui->addItemButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65); color:hsla(52, 100%, 95%, 1);");
        ui->addItemButton->setToolTip("");
        ui->editItemButton->setEnabled(true);
        ui->editItemButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65); color:hsla(52, 100%, 95%, 1);");
        ui->editItemButton->setToolTip("");
        ui->deleteItemButton->setEnabled(true);
        ui->deleteItemButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65); color:hsla(52, 100%, 95%, 1);");
        ui->deleteItemButton->setToolTip("");
        auto okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
        if (okButton) {
            okButton->setEnabled(true);
            okButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65); color:hsla(52, 100%, 95%, 1);");
            okButton->setToolTip("");
        }
    } else {
        query.prepare("UPDATE orders SET customer_ID = :custId, order_date = :orderDate, status = :status, payment_method = :paymentMethod  "
                      "WHERE order_ID = :orderId");
        query.bindValue(":custId", custId);
        query.bindValue(":orderDate", orderDate);
        query.bindValue(":status", statusText);
        query.bindValue(":orderId", orderId);
        query.bindValue(":paymentMethod", paymentMethod);
        if (!query.exec()) {
            QString err = query.lastError().text();
            QMessageBox::warning(this, "Database Error", "Failed to update order: " + err);
            return;
        }
    }

}

void OrderEditor::addItem() {
    if (orderId.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please save the order first.");
        return;
    }

    AddOrderDetailWindow dialog(db, this);  
    if (dialog.exec() == QDialog::Accepted) {
        QSqlRecord record = itemsModel->record();
        record.setValue("order_id", orderId);
        record.setValue("detail_id", dialog.detailId());
        record.setValue("qty", dialog.quantity());
        record.setValue("unit_price", dialog.unitPrice());
        record.setValue("tax_rate", dialog.taxRate());
        record.setValue("sub_total", dialog.subtotal());
        record.setValue("tax_amount", dialog.taxAmount());
        record.setValue("total", dialog.total());
        if (itemsModel->insertRecord(-1, record) && itemsModel->submitAll()) {
            ui->itemsTable->resizeColumnsToContents();
        } else {
            QMessageBox::warning(this, "Error", "Failed to add item: " + itemsModel->lastError().text());
        }   
    }
}

void OrderEditor::editItem() {
    QModelIndexList selection = ui->itemsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select an item to edit.");
        return;
    }
    int row = selection.first().row();
    QSqlRecord record = itemsModel->record(row);
    AddOrderDetailWindow dialog(db, this);
    dialog.setWindowTitle("Edit Order Item");
    dialog.setDetailId(record.value("detail_id").toInt());
    dialog.setQuantity(record.value("qty").toInt());
    dialog.setUnitPrice(record.value("unit_price").toDouble());
    dialog.setTaxRate(record.value("tax_rate").toDouble());
    if (dialog.exec() == QDialog::Accepted) {
        record.setValue("detail_id", dialog.detailId());
        record.setValue("qty", dialog.quantity());
        record.setValue("unit_price", dialog.unitPrice());
        record.setValue("tax_rate", dialog.taxRate());
        record.setValue("sub_total", dialog.subtotal());
        record.setValue("tax_amount", dialog.taxAmount());
        record.setValue("total", dialog.total());
        if (itemsModel->setRecord(row, record) && itemsModel->submitAll()) {
            ui->itemsTable->resizeColumnsToContents();
        } else {
            QMessageBox::warning(this, "Error", "Failed to update item: " + itemsModel->lastError().text());
        }
    }
}

void OrderEditor::deleteItem() {
    QModelIndexList selection = ui->itemsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select an item to delete.");
        return;
    }
    int row = selection.first().row();
    if (QMessageBox::question(this, "Confirm Deletion", "Are you sure you want to delete the selected item?") == QMessageBox::Yes) {
        if (itemsModel->removeRow(row) && itemsModel->submitAll()) {
            ui->itemsTable->resizeColumnsToContents();
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete item: " + itemsModel->lastError().text());
        }
    }
}

void OrderEditor::saveAndClose() {
    if (!itemsModel->submitAll()) {  // NEW: Ensure pending item changes saved
        QMessageBox::warning(this, "Error", "Failed to save order items: " + itemsModel->lastError().text());
        return;
    }
    saveOrderDetails();  
    accept();
}