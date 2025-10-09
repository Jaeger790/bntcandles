// expenseeditor.cpp
#include "expenseeditor.h"
#include "ui_expenseeditor.h"
#include "addExpenseDetailWindow.h"
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

    // Set up headers matching schema (only visible ones for manual entry)
    itemsModel->setHeaderData(1, Qt::Horizontal, "Date");  // date
    itemsModel->setHeaderData(2, Qt::Horizontal, "Description");  // description
    itemsModel->setHeaderData(3, Qt::Horizontal, "Category");  // category
    itemsModel->setHeaderData(5, Qt::Horizontal, "Payment Method");  // payment_method (added for manual)
    itemsModel->setHeaderData(6, Qt::Horizontal, "Source");  // source/merchant
    itemsModel->setHeaderData(7, Qt::Horizontal, "Item Name");  // item_name
    itemsModel->setHeaderData(8, Qt::Horizontal, "Quantity");  // quantity
    itemsModel->setHeaderData(9, Qt::Horizontal, "Subtotal");  // item_subtotal
    itemsModel->setHeaderData(10, Qt::Horizontal, "Tax");  // item_tax
    itemsModel->setHeaderData(11, Qt::Horizontal, "Shipping");  // item_shipping
    itemsModel->setHeaderData(12, Qt::Horizontal, "Promotion");  // item_promotion
    itemsModel->setHeaderData(13, Qt::Horizontal, "Tax Rate");  // item_tax_rate
    itemsModel->setHeaderData(14, Qt::Horizontal, "Notes");  // notes

    // For new order, no filter yet; for edit, filter by order_id
    if (!isNewOrder) {
        itemsModel->setFilter(QString("order_id = %1").arg(orderId));
        loadOrderInfo();
    }
    if (!itemsModel->select()) {
        QMessageBox::warning(this, "Data Error", "Failed to load expense details: " + itemsModel->lastError().text());
    }

    // Configure table view
    ui->itemsTable->setModel(itemsModel);
    ui->itemsTable->hideColumn(0);  // expense_id
    ui->itemsTable->hideColumn(4);  // order_id
    ui->itemsTable->resizeColumnsToContents();
    ui->itemsTable->horizontalHeader()->setStretchLastSection(true);
    ui->itemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->itemsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);  // Read-only table, edits via dialog

    // Default date to today
    ui->dateEdit->setDate(QDate::currentDate());

    // Populate combos
    populateCategoryCombo();

    // Connect signals
    connect(ui->saveOrderButton, &QPushButton::clicked, this, &ExpenseEditor::saveOrderInfo);
    connect(ui->addItemButton, &QPushButton::clicked, this, &ExpenseEditor::addItem);
    connect(ui->editItemButton, &QPushButton::clicked, this, &ExpenseEditor::editItem);
    connect(ui->deleteItemButton, &QPushButton::clicked, this, &ExpenseEditor::deleteItem);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ExpenseEditor::saveAndClose);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ExpenseEditor::reject);

    // For new order: Disable detail ops until order saved
    if (isNewOrder) {
        ui->addItemButton->setEnabled(false);
        ui->editItemButton->setEnabled(false);
        ui->deleteItemButton->setEnabled(false);
        ui->itemsTable->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        ui->saveOrderButton->setVisible(false);  // Hide for edits
    }

    // Ignore Amazon-specific: For manual, we can leave fields like merchant/source optional, but keep for flexibility
    // No separate mode; all fields optional except basics
}

ExpenseEditor::~ExpenseEditor()
{
    delete ui;
    delete itemsModel;
}

void ExpenseEditor::populateCategoryCombo()
{
    // Hardcoded for manual ease; could query DISTINCT category from expense_details
    ui->categoryComboBox->clear();
    ui->categoryComboBox->addItems({"Supplies", "Equipment", "Marketing", "Travel", "Other"});
}

void ExpenseEditor::loadOrderInfo()
{
    QSqlQuery query(db);
    query.prepare("SELECT date, payment_method, source FROM expense_orders WHERE order_id = ?");
    query.bindValue(0, orderId);
    if (query.exec() && query.next()) {
        ui->dateEdit->setDate(query.value(0).toDate());
        ui->paymentMethodComboBox->setCurrentText(query.value(1).toString());
        ui->sourceLineEdit->setText(query.value(2).toString());  // source
    } else {
        qDebug() << "Failed to load order info:" << query.lastError().text();
    }
}

void ExpenseEditor::saveOrderInfo()
{
    // Validate basics for manual entry
    if (ui->dateEdit->date().isNull() || ui->paymentMethodComboBox->currentText().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Date and payment method are required for manual entries.");
        return;
    }

    QSqlQuery query(db);
    if (isNewOrder) {
        // Insert new order
        query.prepare("INSERT INTO expense_orders (date, payment_method, source) VALUES (?, ?, ?)");
        query.addBindValue(ui->dateEdit->date());
        query.addBindValue(ui->paymentMethodComboBox->currentText());
        query.addBindValue(ui->sourceLineEdit->text().trimmed().isEmpty() ? QVariant(QVariant::String) : ui->sourceLineEdit->text().trimmed());
        if (!query.exec()) {
            QMessageBox::warning(this, "DB Error", "Failed to create order: " + query.lastError().text());
            return;
        }
        orderId = QString::number(query.lastInsertId().toInt());
        isNewOrder = false;

        // Reapply filter and select
        itemsModel->setFilter(QString("order_id = %1").arg(orderId));
        itemsModel->select();
    } else {
        // Update existing
        query.prepare("UPDATE expense_orders SET date = ?, payment_method = ?, source = ? WHERE order_id = ?");
        query.addBindValue(ui->dateEdit->date());
        query.addBindValue(ui->paymentMethodComboBox->currentText());
        query.addBindValue(ui->sourceLineEdit->text().trimmed());
        query.addBindValue(orderId);
        if (!query.exec()) {
            QMessageBox::warning(this, "DB Error", "Failed to update order: " + query.lastError().text());
            return;
        }
    }

    // Enable detail management
    ui->addItemButton->setEnabled(true);
    ui->editItemButton->setEnabled(true);
    ui->deleteItemButton->setEnabled(true);
    ui->itemsTable->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    ui->saveOrderButton->setVisible(false);  // Hide after save

    QMessageBox::information(this, "Success", "Order info saved. Now add details for manual transaction.");
}

void ExpenseEditor::saveChanges()
{
    // Submit any pending detail changes (though table is read-only, ensures triggers fire)
    if (!itemsModel->submitAll()) {
        QMessageBox::warning(this, "DB Error", "Failed to save details: " + itemsModel->lastError().text());
        return;
    }
    qDebug() << "Changes saved for order" << orderId;
}

void ExpenseEditor::addItem() { addDetail(); }
void ExpenseEditor::editItem() { editDetail(); }
void ExpenseEditor::deleteItem() { deleteDetail(); }

void ExpenseEditor::addDetail()
{
    if (orderId.isEmpty()) {
        QMessageBox::warning(this, "Error", "Save order info first.");
        return;
    }
    AddExpenseDetailWindow dialog(this);
    dialog.setWindowTitle("Add Manual Expense Detail");
    // Pre-fill for ease: date, category, payment
    dialog.setDate(ui->dateEdit->date().toString(Qt::ISODate));
    dialog.setCategory(ui->categoryComboBox->currentText());
    dialog.setPaymentMethod(ui->paymentMethodComboBox->currentText());
    dialog.setSource(ui->sourceLineEdit->text());
    if (dialog.exec() == QDialog::Accepted) {
        QSqlRecord record = itemsModel->record();
        record.setValue("order_id", orderId.toInt());
        record.setValue("date", dialog.date());
        record.setValue("description", dialog.description());
        record.setValue("category", dialog.category());
        record.setValue("payment_method", dialog.paymentMethod());  // For manual sync
        record.setValue("source", dialog.source());
        record.setValue("item_name", dialog.itemName().isEmpty() ? "Manual Entry" : dialog.itemName());  // Default if empty
        record.setValue("quantity", dialog.quantity());
        record.setValue("item_subtotal", dialog.itemSubtotal());
        record.setValue("item_tax", dialog.itemTax());
        record.setValue("item_shipping", dialog.itemShipping());
        record.setValue("item_promotion", dialog.itemPromotion());
        record.setValue("item_tax_rate", dialog.itemTaxRate());
        record.setValue("notes", dialog.notes());
        if (itemsModel->insertRecord(-1, record) && itemsModel->submitAll()) {
            ui->itemsTable->resizeColumnsToContents();
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
    // Load from record (map indices to fields)
    dialog.setDate(record.value(1).toString());  // date
    dialog.setDescription(record.value(2).toString());
    dialog.setCategory(record.value(3).toString());
    dialog.setPaymentMethod(record.value(5).toString());
    dialog.setSource(record.value(6).toString());
    dialog.setItemName(record.value(7).toString());
    dialog.setQuantity(record.value(8).toInt());
    dialog.setItemSubtotal(record.value(9).toDouble());
    dialog.setItemTax(record.value(10).toDouble());
    dialog.setItemShipping(record.value(11).toDouble());
    dialog.setItemPromotion(record.value(12).toDouble());
    dialog.setItemTaxRate(record.value(13).toDouble());
    dialog.setNotes(record.value(14).toString());
    if (dialog.exec() == QDialog::Accepted) {
        record.setValue("date", dialog.date());
        record.setValue("description", dialog.description());
        record.setValue("category", dialog.category());
        record.setValue("payment_method", dialog.paymentMethod());
        record.setValue("source", dialog.source());
        record.setValue("item_name", dialog.itemName());
        record.setValue("quantity", dialog.quantity());
        record.setValue("item_subtotal", dialog.itemSubtotal());
        record.setValue("item_tax", dialog.itemTax());
        record.setValue("item_shipping", dialog.itemShipping());
        record.setValue("item_promotion", dialog.itemPromotion());
        record.setValue("item_tax_rate", dialog.itemTaxRate());
        record.setValue("notes", dialog.notes());
        if (itemsModel->setRecord(row, record) && itemsModel->submitAll()) {
            ui->itemsTable->resizeColumnsToContents();
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