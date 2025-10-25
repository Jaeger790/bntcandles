#include "../headers/ordereditor.h"
#include "../ui/ui_ordereditor.h"
#include "../headers/addorderdetailwindow.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QSortFilterProxyModel>  // NEW: For proxy model

// NEW: Inner proxy class for displaying product names without changing base model type
class ProductNameProxy : public QSortFilterProxyModel {
public:
    explicit ProductNameProxy(QSqlTableModel *sourceModel, QObject *parent = nullptr)
        : QSortFilterProxyModel(parent), m_sourceModel(sourceModel), m_db(sourceModel->database()) {
        setSourceModel(sourceModel);
    }

    QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole && proxyIndex.column() == 2) {  // Col 2: detail_id -> product_name
            int detailId = sourceModel()->data(sourceModel()->index(proxyIndex.row(), 2), Qt::DisplayRole).toInt();
            if (detailId == 0) return QString("Unknown");

            // Cache check (simple map; for prod, use QCache)
            static QMap<int, QString> nameCache;
            if (nameCache.contains(detailId)) {
                return nameCache[detailId];
            }

            // Query with JOIN to fetch from product table
            QSqlQuery query(m_db);
            query.prepare("SELECT p.product_name FROM product p "
                          "JOIN product_details pd ON p.product_id = pd.product_ID "
                          "WHERE pd.detail_id = :id");
            query.bindValue(":id", detailId);
            if (query.exec() && query.next()) {
                QString name = query.value(0).toString();
                nameCache[detailId] = name;  // Cache
                return name;
            } else {
                return QString("ID: %1").arg(detailId);  // Fallback
            }
        }
        return QSortFilterProxyModel::data(proxyIndex, role);
    }

private:
    QSqlTableModel *m_sourceModel;
    QSqlDatabase m_db;
};

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
        // Load existing for edit - ADDED: payment_method
        QSqlQuery loadQuery(db);
        loadQuery.prepare("SELECT customer_id, status, order_date, payment_method FROM orders WHERE order_id = :orderId");
        loadQuery.bindValue(":orderId", orderId);
        
        // Enable buttons for edit (moved outside any conditional)
        ui->addItemButton->setEnabled(true);
        ui->addItemButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65); color:hsla(52, 100%, 95%, 1);");
        ui->editItemButton->setEnabled(true);
        ui->editItemButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65); color:hsla(52, 100%, 95%, 1);");
        ui->deleteItemButton->setEnabled(true);
        ui->deleteItemButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65); color:hsla(52, 100%, 95%, 1);");
        auto okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
        auto cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);
        if (okButton) {
            okButton->setEnabled(true);
            okButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65); color:hsla(52, 100%, 95%, 1);");
            cancelButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65); color:hsla(52, 100%, 95%, 1);");
        }
        
        if (loadQuery.exec() && loadQuery.next()) {
            int custId = loadQuery.value("customer_id").toInt();
            QString statusId = loadQuery.value("status").toString();
            QDate orderDate = loadQuery.value("order_date").toDate();
            QString paymentMethod = loadQuery.value("payment_method").toString();  // NEW: Load payment

            int custIndex = ui->customerCombo->findData(custId);
            if (custIndex >= 0) {
                ui->customerCombo->setCurrentIndex(custIndex);
            } else {
            }

            int statusIndex = ui->statusCombo->findText(statusId);
            if (statusIndex >= 0) {
                ui->statusCombo->setCurrentIndex(statusIndex);
            } else {
                
                ui->statusCombo->setCurrentIndex(-1);  
                
            }

            // NEW: Set payment
            int payIndex = ui->paymentCombo->findText(paymentMethod);
            if (payIndex >= 0) {
                ui->paymentCombo->setCurrentIndex(payIndex);
            } else {
                ui->paymentCombo->setCurrentIndex(0);
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
    itemsModel->setHeaderData(2, Qt::Horizontal, "Product Name");  // Displays via proxy
    itemsModel->setHeaderData(3, Qt::Horizontal, "Quantity");
    itemsModel->setHeaderData(4, Qt::Horizontal, "Unit Price");
    itemsModel->setHeaderData(5, Qt::Horizontal, "Tax Rate (%)");
    itemsModel->setHeaderData(6, Qt::Horizontal, "Subtotal");
    itemsModel->setHeaderData(7, Qt::Horizontal, "Tax Amount");
    itemsModel->setHeaderData(8, Qt::Horizontal, "Total");
    if (!itemsModel->select()) {
        QMessageBox::warning(this, "Data Error", "Failed to load order items: " + itemsModel->lastError().text());
    }

    // NEW: Wrap with proxy for product name display
    ProductNameProxy *proxyModel = new ProductNameProxy(itemsModel, this);
    ui->itemsTable->setModel(proxyModel);  // Use proxy for view
    ui->itemsTable->setColumnHidden(0, true);
    ui->itemsTable->setColumnHidden(1, true);
    ui->itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
        //make combo boxes blank by default
        ui->customerCombo->setCurrentIndex(-1);
        ui->statusCombo->setCurrentIndex(-1);
        ui->paymentCombo->setCurrentIndex(-1);  // NEW: Blank payment too
        ui->dateEdit->setDate(QDate::currentDate());
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
    }
}

OrderEditor::~OrderEditor() {
    delete itemsModel;
    delete ui;
}

void OrderEditor::populateCustomerCombo() {
    ui->customerCombo->clear();  
    QSqlQuery query(db);
    query.exec("SELECT customer_id, CONCAT(first_name, ' ', last_name) AS name FROM customer ORDER BY name");
    while (query.next()) {
        ui->customerCombo->addItem(query.value("name").toString(), query.value("customer_id"));
    }
}

void OrderEditor::populateStatusCombo() {
    ui->statusCombo->clear();
    QFile file(":/order_statuses.txt");
    QStringList statuses;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Match your ENUM exactly (order matters for consistency, but findText doesn't care)
        statuses << "Pending" << "Paid" << "Shipped" << "Delivered" << "Cancelled" << "Complete";
    } else {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                statuses << line;
            }
        }
        file.close();
    }
    // Add as text-only (no data QVariant)
    foreach (const QString& stat, statuses) {
        ui->statusCombo->addItem(stat);  // Text = value, no data
    }
}

void OrderEditor::populatePaymentCombo(){
    ui->paymentCombo->clear();
    ui->paymentCombo->addItem("Card");
    ui->paymentCombo->addItem("Venmo");
    ui->paymentCombo->addItem("Cash");
    // REMOVED: if (isNewOrder) setCurrentIndex(0); - handled in constructor
}

void OrderEditor::saveOrderDetails() {
    // Validation
    if (ui->customerCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "Validation Error", "Please select a customer.");
        return;
    }
    if (ui->statusCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "Validation Error", "Please select a status.");
        return;
    }
    if(ui->paymentCombo->currentIndex() < 0){
        QMessageBox::warning(this,"Validation Error", "Please select a payment method.");
        return;  // ADDED: return;
    }
    if(!ui->dateEdit->date().isValid()){
        QMessageBox::warning(this,"Validation Error", "Please select a valid date.");
        return;  // ADDED: return;
    }
    int custId = ui->customerCombo->currentData().toInt();
    QString status = ui->statusCombo->currentText();  
    QDate orderDate = ui->dateEdit->date();
    QString paymentMethod = ui->paymentCombo->currentText();
   
    QSqlQuery query(db);
    if (isNewOrder) {
        query.prepare("INSERT INTO orders (customer_id, order_date, status, payment_method) "  // Fixed: Added payment_method to VALUES; fixed casing to match schema
                      "VALUES (:custId, :orderDate, :status, :paymentMethod)");
        query.bindValue(":custId", custId);
        query.bindValue(":orderDate", orderDate);
        query.bindValue(":status", status);
        query.bindValue(":paymentMethod", paymentMethod);
        if (!query.exec()) {
            QString err = query.lastError().text();
            QMessageBox::warning(this, "Database Error", "Failed to create order: " + err);
            return;
        }
        orderId = query.lastInsertId().toString();
        isNewOrder = false;
        // Refresh items filter
        itemsModel->setFilter(QString("order_id = '%1'").arg(orderId));  // Fixed: Consistent casing
        itemsModel->select();  // NEW: Refresh after filter
        // Enable controls (as before)
        ui->addItemButton->setEnabled(true);
        ui->addItemButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65); color:hsla(52, 100%, 95%, 1);");
        ui->addItemButton->setToolTip("Add order before adding details");
        ui->editItemButton->setEnabled(true);
        ui->editItemButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65); color:hsla(52, 100%, 95%, 1);");
        ui->editItemButton->setToolTip("Add order before editing details");
        ui->deleteItemButton->setEnabled(true);
        ui->deleteItemButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65); color:hsla(52, 100%, 95%, 1);");
        ui->deleteItemButton->setToolTip("Add order before deleting items.");
        auto okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
        if (okButton) {
            okButton->setEnabled(true);
            okButton->setStyleSheet("background-color:rgba(255, 134, 215, 0.65); color:hsla(52, 100%, 95%, 1);");
            okButton->setToolTip("");
        }
    } else {
        query.prepare("UPDATE orders SET customer_id = :custId, order_date = :orderDate, status = :status, payment_method = :paymentMethod  "  // Fixed: Casing consistency
                      "WHERE order_id = :orderId");  // Fixed: Casing
        query.bindValue(":custId", custId);
        query.bindValue(":orderDate", orderDate);
        query.bindValue(":status", status);
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
    // Map proxy row to source row
    QSortFilterProxyModel *proxy = qobject_cast<QSortFilterProxyModel*>(ui->itemsTable->model());
    int sourceRow = proxy->mapToSource(selection.first()).row();
    QSqlRecord record = itemsModel->record(sourceRow);
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
        if (itemsModel->setRecord(sourceRow, record) && itemsModel->submitAll()) {
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
    // Map proxy row to source row
    QSortFilterProxyModel *proxy = qobject_cast<QSortFilterProxyModel*>(ui->itemsTable->model());
    int sourceRow = proxy->mapToSource(selection.first()).row();
    if (QMessageBox::question(this, "Confirm Deletion", "Are you sure you want to delete the selected item?") == QMessageBox::Yes) {
        if (itemsModel->removeRow(sourceRow) && itemsModel->submitAll()) {
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