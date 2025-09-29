#include "expenseDetailsWindow.h"
#include "./ui_expenseDetailsWindow.h"
#include "addExpenseDetailWindow.h"
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

ExpenseDetailsWindow::ExpenseDetailsWindow(const QString &orderId, const QSqlDatabase &db, QWidget *parent): QDialog(parent)
    , ui(new Ui::ExpenseDetailsWindow)
    , orderId(orderId)
    , db(db)
{
    ui->setupUi(this);

    // Set order ID label
    ui->orderIdLabel->setText("Order ID: " + orderId);

    // Create and set up model for expense_details
    expenseDetailsModel = new QSqlTableModel(this, db);
    expenseDetailsModel->setTable("expense_details");
    expenseDetailsModel->setFilter(QString("order_id = '%1'").arg(orderId));
    expenseDetailsModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    expenseDetailsModel->setHeaderData(0, Qt::Horizontal, "Expense ID");
    expenseDetailsModel->setHeaderData(1, Qt::Horizontal, "Date");
    expenseDetailsModel->setHeaderData(2, Qt::Horizontal, "Description");
    expenseDetailsModel->setHeaderData(3, Qt::Horizontal, "Category");
    expenseDetailsModel->setHeaderData(4, Qt::Horizontal, "Order ID");
    expenseDetailsModel->setHeaderData(5, Qt::Horizontal, "Payment Method");
    expenseDetailsModel->setHeaderData(6, Qt::Horizontal, "Source");
    expenseDetailsModel->setHeaderData(7, Qt::Horizontal, "Item Name");
    expenseDetailsModel->setHeaderData(8, Qt::Horizontal, "Quantity");
    expenseDetailsModel->setHeaderData(9, Qt::Horizontal, "Subtotal");
    expenseDetailsModel->setHeaderData(10, Qt::Horizontal, "Tax");
    expenseDetailsModel->setHeaderData(11, Qt::Horizontal, "Shipping");
    expenseDetailsModel->setHeaderData(12, Qt::Horizontal, "Promotion");
    expenseDetailsModel->setHeaderData(13, Qt::Horizontal, "Tax Rate");
    expenseDetailsModel->setHeaderData(14, Qt::Horizontal, "Notes");
    if (!expenseDetailsModel->select()) {
        qDebug() << "Expense details model error:" << expenseDetailsModel->lastError().text();
        QMessageBox::warning(this, "Data Error", "Failed to load expense details: " + expenseDetailsModel->lastError().text());
    }

    ui->detailsTable->setModel(expenseDetailsModel);
    ui->detailsTable->setColumnHidden(0, true); // Hide expense_id
    ui->detailsTable->setColumnHidden(4, true); // Hide order_id
    ui->detailsTable->resizeColumnsToContents();
    ui->detailsTable->horizontalHeader()->setStretchLastSection(true);
    ui->detailsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->detailsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Connect buttons
    connect(ui->addDetailButton, &QPushButton::clicked, this, &ExpenseDetailsWindow::addDetail);
    connect(ui->editDetailButton, &QPushButton::clicked, this, &ExpenseDetailsWindow::editDetail);
    connect(ui->deleteDetailButton, &QPushButton::clicked, this, &ExpenseDetailsWindow::deleteDetail);
}

ExpenseDetailsWindow::~ExpenseDetailsWindow()
{
    delete expenseDetailsModel;
    delete ui;
}

void ExpenseDetailsWindow::addDetail()
{
    AddExpenseDetailWindow dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QSqlRecord record = expenseDetailsModel->record();
        record.setValue("order_id", orderId);
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
        if (expenseDetailsModel->insertRecord(-1, record)) {
            if (expenseDetailsModel->submitAll()) {
                ui->detailsTable->resizeColumnsToContents();
            } else {
                QMessageBox::warning(this, "Error", "Failed to add expense detail: " + expenseDetailsModel->lastError().text());
            }
        } else {
            QMessageBox::warning(this, "Error", "Failed to insert expense detail record: " + expenseDetailsModel->lastError().text());
        }
    }
}

void ExpenseDetailsWindow::editDetail()
{
    QModelIndexList selection = ui->detailsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select an expense detail to edit.");
        return;
    }
    int row = selection.first().row();
    QSqlRecord record = expenseDetailsModel->record(row);
    AddExpenseDetailWindow dialog(this);
    dialog.setWindowTitle("Edit Expense Detail");
    dialog.setDate(record.value("date").toString());
    dialog.setDescription(record.value("description").toString());
    dialog.setCategory(record.value("category").toString());
    dialog.setPaymentMethod(record.value("payment_method").toString());
    dialog.setSource(record.value("source").toString());
    dialog.setItemName(record.value("item_name").toString());
    dialog.setQuantity(record.value("quantity").toInt());
    dialog.setItemSubtotal(record.value("item_subtotal").toDouble());
    dialog.setItemTax(record.value("item_tax").toDouble());
    dialog.setItemShipping(record.value("item_shipping").toDouble());
    dialog.setItemPromotion(record.value("item_promotion").toDouble());
    dialog.setItemTaxRate(record.value("item_tax_rate").toDouble());
    dialog.setNotes(record.value("notes").toString());
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
        if (expenseDetailsModel->setRecord(row, record)) {
            if (expenseDetailsModel->submitAll()) {
                ui->detailsTable->resizeColumnsToContents();
            } else {
                QMessageBox::warning(this, "Error", "Failed to save changes: " + expenseDetailsModel->lastError().text());
            }
        } else {
            QMessageBox::warning(this, "Error", "Failed to update expense detail record: " + expenseDetailsModel->lastError().text());
        }
    }
}

void ExpenseDetailsWindow::deleteDetail()
{
    QModelIndexList selection = ui->detailsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select an expense detail to delete.");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete the selected expense detail?",
        QMessageBox::Yes | QMessageBox::No
        );
    if (reply == QMessageBox::Yes) {
        for (const auto& index : selection) {
            expenseDetailsModel->removeRow(index.row());
        }
        if (expenseDetailsModel->submitAll()) {
            ui->detailsTable->resizeColumnsToContents();
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete expense detail: " + expenseDetailsModel->lastError().text());
        }
    }
}
