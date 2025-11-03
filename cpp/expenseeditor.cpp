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
#include <QLocale>

ExpenseEditor::ExpenseEditor(const QString &orderId, const QSqlDatabase &db, QWidget *parent)
    : QDialog(parent), ui(new Ui::ExpenseEditor), orderId(orderId), db(db), isNewOrder(orderId.isEmpty())
{
    ui->setupUi(this);
    setWindowTitle(isNewOrder ? "Add New Expense Order" : "Edit Expense Order");

    // Setup model
    itemsModel = new QSqlTableModel(this, db);
    itemsModel->setTable("expense_details");
    itemsModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Headers (trimmed to single Qt::Horizontal call)
    itemsModel->setHeaderData(1, Qt::Horizontal, "Description");
    itemsModel->setHeaderData(2, Qt::Horizontal, "Category");
    itemsModel->setHeaderData(4, Qt::Horizontal, "Date");
    itemsModel->setHeaderData(5, Qt::Horizontal, "City");  // Merchant
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
    ui->itemsTable->hideColumn(0);   // expense_id
    ui->itemsTable->hideColumn(3);   // order_id
    ui->itemsTable->hideColumn(15);  // created_at
    ui->itemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->itemsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->dateEdit->setDate(QDate::currentDate());
    populateCategoryCombo();
    populatePaymentMethodCombo();

    // Connections
    connect(ui->saveOrderButton, &QPushButton::clicked, this, &ExpenseEditor::saveOrderInfo);
    connect(ui->addItemButton, &QPushButton::clicked, this, &ExpenseEditor::addItem);
    connect(ui->editItemButton, &QPushButton::clicked, this, &ExpenseEditor::editItem);
    connect(ui->deleteItemButton, &QPushButton::clicked, this, &ExpenseEditor::deleteItem);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ExpenseEditor::saveAndClose);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ExpenseEditor::reject);

    // Live total updates (single emit post-DML via explicit queries)
    connect(itemsModel, &QSqlTableModel::rowsInserted, this, &ExpenseEditor::updateRunningTotal);
    connect(itemsModel, &QSqlTableModel::rowsRemoved, this, &ExpenseEditor::updateRunningTotal);
    connect(itemsModel, &QSqlTableModel::dataChanged, this, &ExpenseEditor::updateRunningTotal);

    updateUIState();
    updateRunningTotal();  // Initial
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
        if (ui->merchantLineEdit) ui->merchantLineEdit->setText(query.value(2).toString());

        // Load DB total for edit (optional UI sync)
        QSqlQuery totalQuery(db);
        totalQuery.prepare("SELECT order_total FROM expense_orders WHERE order_id = ?");
        totalQuery.bindValue(0, orderId);
        if (totalQuery.exec() && totalQuery.next()) {
            qDebug() << "Loaded DB total for edit:" << totalQuery.value(0).toDouble() << "for order" << orderId;
            // ui->totalLabel->setText(QString("Grand Total: $%1").arg(totalQuery.value(0).toDouble(), 0, 'f', 2));
        }
    } else {
        qDebug() << "Load order info failed:" << query.lastError().text();
    }
}

void ExpenseEditor::saveOrderInfo()
{
    if (ui->dateEdit->date().isNull() || ui->paymentMethodComboBox->currentText().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Date and payment method required.");
        return;
    }

    QSqlQuery query(db);
    if (isNewOrder) {
        query.prepare("INSERT INTO expense_orders (date, payment_method, merchant) VALUES (?, ?, ?)");
        query.addBindValue(ui->dateEdit->date());
        query.addBindValue(ui->paymentMethodComboBox->currentText());
        query.addBindValue(ui->merchantLineEdit ? ui->merchantLineEdit->text().trimmed() : "");  // Null-safe
        if (!query.exec()) {
            QMessageBox::warning(this, "DB Error", "Failed to create order: " + query.lastError().text());
            return;
        }
        orderId = QString::number(query.lastInsertId().toInt());
        if (orderId.isEmpty()) {
            QMessageBox::warning(this, "DB Error", "Failed to retrieve new order ID.");
            return;
        }
        isNewOrder = false;
        itemsModel->setFilter(QString("order_id = %1").arg(orderId));
        itemsModel->select();
    } else {
        query.prepare("UPDATE expense_orders SET date = ?, payment_method = ?, merchant = ? WHERE order_id = ?");
        query.addBindValue(ui->dateEdit->date());
        query.addBindValue(ui->paymentMethodComboBox->currentText());
        query.addBindValue(ui->merchantLineEdit ? ui->merchantLineEdit->text().trimmed() : "");
        query.addBindValue(orderId);
        if (!query.exec()) {
            QMessageBox::warning(this, "DB Error", "Failed to update order: " + query.lastError().text());
            return;
        }
    }

    updateUIState();
    QMessageBox::information(this, "Success", "Order saved. Add details now.");
}

void ExpenseEditor::saveChanges()
{
    if (!itemsModel->submitAll()) {
        qDebug() << "Save changes failed:" << itemsModel->lastError().text();
        QMessageBox::warning(this, "DB Error", "Failed to save details: " + itemsModel->lastError().text());
        return;
    }

    // Verify trigger
    QSqlQuery verify(db);
    verify.prepare("SELECT order_total FROM expense_orders WHERE order_id = ?");
    verify.bindValue(0, orderId);
    if (verify.exec() && verify.next()) {
        qDebug() << "Post-save DB total:" << verify.value(0).toDouble() << "for order" << orderId;
    }

    refreshTable();
}

ExpenseEditor::OrderTotals ExpenseEditor::calculateOrderTotals() const
{
    OrderTotals totals;
    for (int row = 0; row < itemsModel->rowCount(); ++row) {
        totals.subtotal += itemsModel->record(row).value("item_subtotal").toDouble();  // Named for clarity
        totals.tax += itemsModel->record(row).value("item_tax").toDouble();
        totals.shipping += itemsModel->record(row).value("item_shipping").toDouble();
        totals.promotion += itemsModel->record(row).value("item_promotion").toDouble();
    }
    return totals;
}

void ExpenseEditor::updateRunningTotal()
{
    OrderTotals totals = calculateOrderTotals();
    QString totalText = QLocale::system().toCurrencyString(totals.grandTotal());
    // ui->totalLabel->setText(QString("Grand Total: %1").arg(totalText));
    qDebug() << "Running total for" << orderId << ":" << totals.grandTotal() << "(" << totalText << ")";
}

void ExpenseEditor::saveAndClose()
{
    saveChanges();
    if (!itemsModel->lastError().isValid()) {
        accept();
    }
}

void ExpenseEditor::populateRecordFromDialog(QSqlRecord &record, const AddExpenseDetailWindow &dialog, bool isEdit)
{
    if (isEdit) {
        record.setValue("order_id", orderId.toInt());
    }
    record.setValue("description", dialog.description());
    record.setValue("category", dialog.category());
    record.setValue("date", QDate::fromString(dialog.date(), Qt::ISODate));
    record.setValue("merchant", dialog.merchant());
    record.setValue("item_name", dialog.itemName().isEmpty() ? "Manual Entry" : dialog.itemName());
    record.setValue("quantity", dialog.quantity());
    record.setValue("unit_price", dialog.unitPrice());
    record.setValue("item_subtotal", dialog.itemSubtotal());
    record.setValue("item_tax", dialog.itemTax());
    record.setValue("item_shipping", dialog.itemShipping());
    record.setValue("item_promotion", dialog.itemPromotion());
    record.setValue("item_tax_rate", dialog.itemTaxRate() / 100.0);
    record.setValue("notes", dialog.notes());
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
    dialog.setMerchant(ui->merchantLineEdit ? ui->merchantLineEdit->text() : "");  // Null-safe
    if (dialog.exec() != QDialog::Accepted || dialog.unitPrice() <= 0) {
        if (dialog.unitPrice() <= 0) {
            QMessageBox::warning(this, "Validation Error", "Unit price must be > 0.");
        }
        return;
    }

    // Explicit INSERT
    QSqlQuery query(db);
    query.prepare("INSERT INTO expense_details (order_id, description, category, date, merchant, item_name, quantity, unit_price, item_subtotal, item_tax, item_shipping, item_promotion, item_tax_rate, notes) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(orderId.toInt());
    query.addBindValue(dialog.description());
    query.addBindValue(dialog.category());
    query.addBindValue(QDate::fromString(dialog.date(), Qt::ISODate));
    query.addBindValue(dialog.merchant());
    query.addBindValue(dialog.itemName().isEmpty() ? "Manual Entry" : dialog.itemName());
    query.addBindValue(dialog.quantity());
    query.addBindValue(dialog.unitPrice());
    query.addBindValue(dialog.itemSubtotal());
    query.addBindValue(dialog.itemTax());
    query.addBindValue(dialog.itemShipping());
    query.addBindValue(dialog.itemPromotion());
    query.addBindValue(dialog.itemTaxRate() / 100.0);
    query.addBindValue(dialog.notes());

    if (!query.exec()) {
        qDebug() << "Add detail failed:" << query.lastError().text();
        QMessageBox::warning(this, "Error", "Failed to add detail: " + query.lastError().text());
        return;
    }

    qDebug() << "Added detail - new expense_id:" << query.lastInsertId();

    // Sync model
    itemsModel->select();
    updateRunningTotal();
    refreshTable();
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
    int expenseId = record.value(0).toInt();
    qDebug() << "Editing expense_id:" << expenseId << "for order" << orderId;

    AddExpenseDetailWindow dialog(this);
    dialog.setWindowTitle("Edit Expense Detail");
    dialog.setDate(record.value(4).toString());
    dialog.setDescription(record.value(1).toString());
    dialog.setCategory(record.value(2).toString());
    dialog.setMerchant(record.value(5).toString());
    dialog.setItemName(record.value(6).toString());
    dialog.setQuantity(record.value(7).toInt());
    dialog.setUnitPrice(record.value(8).toDouble());
    dialog.setItemTaxRate(record.value(13).toDouble() * 100.0);
    dialog.setNotes(record.value(14).toString());

    if (dialog.exec() != QDialog::Accepted || dialog.unitPrice() <= 0) {
        if (dialog.unitPrice() <= 0) {
            QMessageBox::warning(this, "Validation Error", "Unit price must be > 0.");
        }
        return;
    }

    // Explicit UPDATE
    QSqlQuery query(db);
    query.prepare("UPDATE expense_details SET description = ?, category = ?, date = ?, merchant = ?, item_name = ?, quantity = ?, unit_price = ?, item_subtotal = ?, item_tax = ?, item_shipping = ?, item_promotion = ?, item_tax_rate = ?, notes = ? WHERE expense_id = ?");
    query.addBindValue(dialog.description());
    query.addBindValue(dialog.category());
    query.addBindValue(QDate::fromString(dialog.date(), Qt::ISODate));
    query.addBindValue(dialog.merchant());
    query.addBindValue(dialog.itemName().isEmpty() ? "Manual Entry" : dialog.itemName());
    query.addBindValue(dialog.quantity());
    query.addBindValue(dialog.unitPrice());
    query.addBindValue(dialog.itemSubtotal());
    query.addBindValue(dialog.itemTax());
    query.addBindValue(dialog.itemShipping());
    query.addBindValue(dialog.itemPromotion());
    query.addBindValue(dialog.itemTaxRate() / 100.0);
    query.addBindValue(dialog.notes());
    query.addBindValue(expenseId);

    if (!query.exec()) {
        qDebug() << "Edit detail failed:" << query.lastError().text();
        QMessageBox::warning(this, "Error", "Failed to update detail: " + query.lastError().text());
        return;
    }

    qDebug() << "Updated detail - rows affected:" << query.numRowsAffected();

    // Sync model
    itemsModel->select();
    updateRunningTotal();
    refreshTable();
}

void ExpenseEditor::deleteDetail()
{
    QModelIndexList selection = ui->itemsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Select a detail to delete.");
        return;
    }

    QString confirmText = selection.size() > 1 ? "Delete selected details?" : "Delete selected detail?";
    if (QMessageBox::question(this, "Confirm", confirmText) != QMessageBox::Yes) {
        return;
    }

    QStringList expenseIds;
    for (const auto& idx : selection) {
        int row = idx.row();
        int expenseId = itemsModel->record(row).value(0).toInt();
        expenseIds << QString::number(expenseId);
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM expense_details WHERE expense_id = ?");
    int deleted = 0;
    for (const QString& idStr : expenseIds) {
        query.bindValue(0, idStr.toInt());
        if (query.exec()) {
            deleted += query.numRowsAffected();
        } else {
            qDebug() << "Delete failed for id" << idStr << ":" << query.lastError().text();
            QMessageBox::warning(this, "Error", "Failed to delete some details: " + query.lastError().text());
            return;
        }
    }

    qDebug() << "Deleted" << deleted << "details";

    if (deleted == 0) {
        QMessageBox::warning(this, "No Change", "No details deleted.");
        return;
    }

    // Sync model
    itemsModel->select();
    updateRunningTotal();
    refreshTable();
}

void ExpenseEditor::updateUIState()
{
    bool itemsEnabled = !isNewOrder;
    ui->addItemButton->setEnabled(itemsEnabled);
    ui->editItemButton->setEnabled(itemsEnabled);
    ui->deleteItemButton->setEnabled(itemsEnabled);
    ui->itemsTable->setEnabled(itemsEnabled);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(itemsEnabled);

    // Save always enabled
    ui->saveOrderButton->setEnabled(true);

    QString activeStyle = "background-color: hsla(319, 30%, 16%, .1);";
    if (itemsEnabled) {
        ui->addItemButton->setStyleSheet(activeStyle);
        ui->editItemButton->setStyleSheet(activeStyle);
        ui->deleteItemButton->setStyleSheet(activeStyle);
        ui->itemsTable->setStyleSheet(activeStyle);
    } else {
        ui->addItemButton->setStyleSheet("");
        ui->editItemButton->setStyleSheet("");
        ui->deleteItemButton->setStyleSheet("");
        ui->itemsTable->setStyleSheet("");
    }
    ui->saveOrderButton->setStyleSheet(activeStyle);
}

void ExpenseEditor::refreshTable()
{
    ui->itemsTable->resizeColumnsToContents();
    ui->itemsTable->horizontalHeader()->setStretchLastSection(true);
}