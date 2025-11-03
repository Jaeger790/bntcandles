#include "../headers/orderpage.h"
#include "../ui/ui_orderpage.h"
#include "../headers/ordereditor.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>  // Added: For lastError().text()

OrderPage::OrderPage(const QSqlDatabase &db, QWidget *parent)
    : QWidget(parent), ui(new Ui::OrderPage), db(db)  // Fixed: Init list with members
{
    ui->setupUi(this);

    orderModel = new QSqlTableModel(this, db);  // Now declared
    orderModel->setTable("ordersview");
    orderModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    orderModel->setHeaderData(0, Qt::Horizontal, "Order ID");
    orderModel->setHeaderData(1, Qt::Horizontal, "First Name");
    orderModel->setHeaderData(2, Qt::Horizontal, "Last Name");
    orderModel->setHeaderData(3, Qt::Horizontal, "Order Date");
    orderModel->setHeaderData(4, Qt::Horizontal, "Status");
    orderModel->setHeaderData(5, Qt::Horizontal, "Subtotal");
    orderModel->setHeaderData(6, Qt::Horizontal, "Tax Amount");
    orderModel->setHeaderData(7, Qt::Horizontal, "Grand Total");
    orderModel->setHeaderData(8, Qt::Horizontal, "Payment Method");

    setupOrderTable();
    refreshTable();  // Now declared

    // Connect buttons
    connect(ui->addOrderButton, &QPushButton::clicked, this, &OrderPage::addOrder);
    connect(ui->editOrderButton, &QPushButton::clicked, this, &OrderPage::editOrder);
    connect(ui->deleteOrderButton, &QPushButton::clicked, this, &OrderPage::deleteOrder);

    // Double-click to edit (Fixed: capture &db; ui/orderModel via this)
    connect(ui->orderTable, &QTableView::doubleClicked, this, [this, &db](const QModelIndex &index) {
        ui->orderTable->selectRow(index.row());
        QString orderId = orderModel->data(orderModel->index(index.row(), 0)).toString();
        OrderEditor editor(orderId, db, this);
        if (editor.exec() == QDialog::Accepted) {
            refreshTable();
        }
    });
}

OrderPage::~OrderPage()
{
    delete ui;
    delete orderModel;
}

void OrderPage::setupOrderTable()
{
    if (!orderModel->select()) {
        QMessageBox::warning(this, "Data Error", "Failed to load orders: " + orderModel->lastError().text());
        return;
    }
    ui->orderTable->setModel(orderModel);
    ui->orderTable->setColumnHidden(0, false); // Show order_id
    ui->orderTable->resizeColumnsToContents();
    ui->orderTable->horizontalHeader()->setStretchLastSection(false);
    ui->orderTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->orderTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->orderTable->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void OrderPage::refreshTable()  // Added: Full impl
{
    orderModel->select(); // Force refresh
    ui->orderTable->resizeColumnsToContents();
}

void OrderPage::addOrder()
{
    OrderEditor editor("", db, this); // Empty ID for add
    if (editor.exec() == QDialog::Accepted) {
        refreshTable();
    }
}

void OrderPage::editOrder()
{
    QModelIndexList selection = ui->orderTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select an order to edit.");
        return;
    }
    int row = selection.first().row();
    QString orderId = orderModel->data(orderModel->index(row, 0)).toString();
    OrderEditor editor(orderId, db, this);
    if (editor.exec() == QDialog::Accepted) {
        refreshTable();
    }
}

void OrderPage::deleteOrder()
{
    QModelIndexList selection = ui->orderTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select an order to delete.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete the selected order(s)? This will also delete associated order details.",
        QMessageBox::Yes | QMessageBox::No
    );
    if (reply != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query(db);
    bool success = true;

    for (const auto &index : selection) {  // No sort needed (deleting via ID, not rows)
        int orderId = orderModel->data(orderModel->index(index.row(), 0)).toInt();

        // First, delete associated order details
        query.prepare("DELETE FROM order_items WHERE order_ID = :orderId");
        query.bindValue(":orderId", orderId);
        if (!query.exec()) {
            success = false;
            QMessageBox::warning(this, "Database Error", "Failed to delete order details: " + query.lastError().text());
            break;
        }

        // Then, delete the order
        query.prepare("DELETE FROM `orders` WHERE order_ID = :orderId");
        query.bindValue(":orderId", orderId);
        if (!query.exec()) {
            success = false;
            QMessageBox::warning(this, "Database Error", "Failed to delete order: " + query.lastError().text());
            break;
        }
    }

    if (success) {
        refreshTable();
    }
}