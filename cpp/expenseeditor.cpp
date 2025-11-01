// expenseeditor.cpp
#include "../headers/expenseeditor.h"
#include "../ui/ui_expenseeditor.h"
#include "../headers/addexpensedetailwindow.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QDate>

ExpenseEditor::ExpenseEditor(const QString &orderId, const QSqlDatabase &db, QWidget *parent)
    : QDialog(parent), ui(new Ui::ExpenseEditor), orderId(orderId), db(db), isNewOrder(orderId.isEmpty())
{
    ui->setupUi(this);
    setWindowTitle(isNewOrder ? "Add New Expense Order" : "Edit Expense Order");

    // Setup model for expense_details table
    itemsModel = new QSqlTableModel(this, db);
    itemsModel->setTable("expense_details");
    itemsModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Headers with CORRECT schema cols (post-unit_price): 0:id,1:desc,2:cat,3:order_id,4:date,5:merchant,6:item_name,7:qty,8:unit_price,9:subtotal,10:tax,11:shipping,12:prom,13:tax_rate,14:notes,15:created_at
    itemsModel->setHeaderData(1, Qt::Horizontal, "Description");
    itemsModel->setHeaderData(2, Qt::Horizontal, "Category");
    itemsModel->setHeaderData(4, Qt::Horizontal, "Date");
    itemsModel->setHeaderData(5, Qt::Horizontal, "Merchant");
    itemsModel->setHeaderData(6, Qt::Horizontal, "Item Name");
    itemsModel->setHeaderData(7, Qt::Horizontal, "Quantity");
    itemsModel->setHeaderData(8, Qt::Horizontal, "Unit Price");
    itemsModel->setHeaderData(9, Qt::Horizontal, "Subtotal");
    itemsModel->setHeaderData(10, Qt::Horizontal, "Tax");
    itemsModel->setHeaderData(11, Qt::Horizontal, "Shipping");
    itemsModel->setHeaderData(12, Qt::Horizontal, "Promotion");
    itemsModel->setHeaderData(13, Qt::Horizontal, "Tax Rate");
    itemsModel->setHeaderData(14, Qt::Horizontal, "Notes");

    if (!isNewOrder) {
        itemsModel->setFilter(QString("order_id = %1").arg(orderId));
        loadOrderInfo();
    }
    if (!itemsModel->select()) {
        QMessageBox::warning(this, "Data Error", "Failed to load expense details: " + itemsModel->lastError().text());
    }

    // Configure table
    ui->itemsTable->setModel(itemsModel);
    ui->itemsTable->hideColumn(0);   // id
    ui->itemsTable->hideColumn(3);   // order_id
    ui->itemsTable->hideColumn(15);  // created_at
    ui->itemsTable->resizeColumnsToContents();
    ui->itemsTable->horizontalHeader()->setStretchLastSection(true);
    ui->itemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->itemsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->dateEdit->setDate(QDate::currentDate());

    populateCategoryCombo();
    populatePaymentMethodCombo();

    connect(ui->saveOrderButton, &QPushButton::clicked, this, &ExpenseEditor::saveOrderInfo);
    connect(ui->addItemButton, &QPushButton::clicked, this, &ExpenseEditor::addItem);
    connect(ui->editItemButton, &QPushButton::clicked, this, &ExpenseEditor::editItem);
    connect(ui->deleteItemButton, &QPushButton::clicked, this, &ExpenseEditor::deleteItem);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ExpenseEditor::saveAndClose);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ExpenseEditor::reject);

    if (isNewOrder) {
        ui->addItemButton->setEnabled(false);
        ui->editItemButton->setEnabled(false);
        ui->deleteItemButton->setEnabled(false);
        ui->itemsTable->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        ui->saveOrderButton->setVisible(false);
    }
}

ExpenseEditor::~ExpenseEditor()
{
    delete ui;
    delete itemsModel;
}

void ExpenseEditor::populateCategoryCombo()
{
    ui->categoryComboBox->clear();
    ui->categoryComboBox->addItems({"Supplies", "Equipment", "Marketing", "Travel", "Meals", "Other"});
}

void ExpenseEditor::populatePaymentMethodCombo()
{
    ui->paymentMethodComboBox->clear();
    ui->paymentMethodComboBox->addItems({"Cash", "Card", "Venmo", "Check", "Other"});
}

void ExpenseEditor::loadOrderInfo()
{
    QSqlQuery query(db);
    query.prepare("SELECT date, payment_method, merchant FROM expense_orders WHERE order_id = ?");
    query.bindValue(0, orderId);
    if (query.exec() && query.next()) {
        ui->dateEdit->setDate(query.value(0).toDate());
        ui->paymentMethodComboBox->setCurrentText(query.value(1).toString());
        ui->merchantLineEdit->setText(query.value(2).toString());
    } else {
        qDebug() << "Failed to load order info:" << query.lastError().text();
    }
}

void ExpenseEditor::saveOrderInfo()
{
    if (ui->dateEdit->date().isNull() || ui->paymentMethodComboBox->currentText().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Date and payment method are required.");
        return;
    }

    QSqlQuery query(db);
    if (isNewOrder) {
        query.prepare("INSERT INTO expense_orders (date, payment_method, merchant) VALUES (?, ?, ?)");
        query.addBindValue(ui->dateEdit->date());
        query.addBindValue(ui->paymentMethodComboBox->currentText());
        query.addBindValue(ui->merchantLineEdit->text().trimmed().isEmpty() ? QVariant() : ui->merchantLineEdit->text().trimmed());
        if (!query.exec()) {
            QMessageBox::warning(this, "DB Error", "Failed to create order: " + query.lastError().text());
            return;
        }
        orderId = QString::number(query.lastInsertId().toInt());
        isNewOrder = false;
        itemsModel->setFilter(QString("order_id = %1").arg(orderId));
        itemsModel->select();
    } else {
        query.prepare("UPDATE expense_orders SET date = ?, payment_method = ?, merchant = ? WHERE order_id = ?");
        query.addBindValue(ui->dateEdit->date());
        query.addBindValue(ui->paymentMethodComboBox->currentText());
        query.addBindValue(ui->merchantLineEdit->text().trimmed());
        query.addBindValue(orderId);
        if (!query.exec()) {
            QMessageBox::warning(this, "DB Error", "Failed to update order: " + query.lastError().text());
            return;
        }
    }

    ui->addItemButton->setEnabled(true);
    ui->editItemButton->setEnabled(true);
    ui->deleteItemButton->setEnabled(true);
    ui->itemsTable->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    ui->saveOrderButton->setVisible(false);

    QMessageBox::information(this, "Success", "Order saved. Add details now.");
}

void ExpenseEditor::saveChanges()
{
    if (!itemsModel->submitAll()) {
        QMessageBox::warning(this, "DB Error", "Failed to save details: " + itemsModel->lastError().text());
        return;
    }
    qDebug() << "Changes saved for order" << orderId;
}

void ExpenseEditor::addDetail()
{
    if (orderId.isEmpty()) {
        QMessageBox::warning(this, "Error", "Save order info first.");
        return;
    }
    AddExpenseDetailWindow dialog(this);
    dialog.setDate(ui->dateEdit->date().toString(Qt::ISODate));
    dialog.setCategory(ui->categoryComboBox->currentText());
    dialog.setMerchant(ui->merchantLineEdit->text());
    if (dialog.exec() == QDialog::Accepted) {
        if (dialog.unitPrice() <= 0) {
            QMessageBox::warning(this, "Validation Error", "Unit price must be > 0.");
            return;
        }
        QSqlRecord record = itemsModel->record();
        record.setValue("order_id", orderId.toInt());  // col3
        record.setValue("description", dialog.description());  // col1
        record.setValue("category", dialog.category());  // col2
        record.setValue("date", dialog.date());  // col4
        record.setValue("merchant", dialog.merchant());  // col5
        record.setValue("item_name", dialog.itemName().isEmpty() ? "Manual Entry" : dialog.itemName());  // col6
        record.setValue("quantity", dialog.quantity());  // col7
        record.setValue("unit_price", dialog.unitPrice());  // col8
        record.setValue("item_subtotal", dialog.itemSubtotal());  // col9
        record.setValue("item_tax", dialog.itemTax());  // col10
        record.setValue("item_shipping", dialog.itemShipping());  // col11 = 0
        record.setValue("item_promotion", dialog.itemPromotion());  // col12 = 0
        record.setValue("item_tax_rate", dialog.itemTaxRate() / 100.0);  // col13, decimal
        record.setValue("notes", dialog.notes());  // col14
        if (itemsModel->insertRecord(-1, record) && itemsModel->submitAll()) {
            ui->itemsTable->resizeColumnsToContents();
            qDebug() << "Added detail; subtotal:" << dialog.itemSubtotal();
        } else {
            QMessageBox::warning(this, "Error", "Failed to add detail: " + itemsModel->lastError().text());
        }
    }
}

void ExpenseEditor::editDetail()
{
    QModelIndexList selection = ui->itemsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Select a detail to edit.");
        return;
    }
    int row = selection.first().row();
    QSqlRecord record = itemsModel->record(row);
    AddExpenseDetailWindow dialog(this);
    dialog.setWindowTitle("Edit Expense Detail");
    dialog.setDate(record.value(4).toString());  // col4
    dialog.setDescription(record.value(1).toString());  // col1
    dialog.setCategory(record.value(2).toString());  // col2
    dialog.setMerchant(record.value(5).toString());  // col5
    dialog.setItemName(record.value(6).toString());  // col6
    dialog.setQuantity(record.value(7).toInt());  // col7
    dialog.setUnitPrice(record.value(8).toDouble());  // col8
    dialog.setItemTaxRate(record.value(13).toDouble() * 100.0);  // col13 *100 to %
    dialog.setNotes(record.value(14).toString());  // col14
    if (dialog.exec() == QDialog::Accepted) {
        if (dialog.unitPrice() <= 0) {
            QMessageBox::warning(this, "Validation Error", "Unit price must be > 0.");
            return;
        }
        record.setValue("description", dialog.description());  // col1
        record.setValue("category", dialog.category());  // col2
        record.setValue("date", dialog.date());  // col4
        record.setValue("merchant", dialog.merchant());  // col5
        record.setValue("item_name", dialog.itemName());  // col6
        record.setValue("quantity", dialog.quantity());  // col7
        record.setValue("unit_price", dialog.unitPrice());  // col8
        record.setValue("item_subtotal", dialog.itemSubtotal());  // col9
        record.setValue("item_tax", dialog.itemTax());  // col10
        record.setValue("item_shipping", dialog.itemShipping());  // col11
        record.setValue("item_promotion", dialog.itemPromotion());  // col12
        record.setValue("item_tax_rate", dialog.itemTaxRate() / 100.0);  // col13
        record.setValue("notes", dialog.notes());  // col14
        if (itemsModel->setRecord(row, record) && itemsModel->submitAll()) {
            ui->itemsTable->resizeColumnsToContents();
            qDebug() << "Updated detail; new subtotal:" << dialog.itemSubtotal();
        } else {
            QMessageBox::warning(this, "Error", "Failed to update detail: " + itemsModel->lastError().text());
        }
    }
}

void ExpenseEditor::deleteDetail()
{
    QModelIndexList selection = ui->itemsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Select details to delete.");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm", "Delete selected details?");
    if (reply == QMessageBox::Yes) {
        for (const auto& idx : selection) {
            itemsModel->removeRow(idx.row());
        }
        if (itemsModel->submitAll()) {
            ui->itemsTable->resizeColumnsToContents();
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete: " + itemsModel->lastError().text());
        }
    }
}

void ExpenseEditor::saveAndClose()
{
    saveChanges();
    if (!itemsModel->lastError().isValid()) {
        accept();
    }
}

void ExpenseEditor::addItem() { addDetail(); }
void ExpenseEditor::editItem() { editDetail(); }
void ExpenseEditor::deleteItem() { deleteDetail(); }