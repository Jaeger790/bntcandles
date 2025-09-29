#include "orderdetailswindow.h"
#include "./ui_orderdetailswindow.h"
#include "addOrderDetailWindow.h"
#include <QSqlRelationalTableModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>

OrderDetailsWindow::OrderDetailsWindow(const QString &orderId, const QSqlDatabase &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OrderDetailsWindow)
    , orderId(orderId)
    , db(db)
{
    ui->setupUi(this);

    // Query customer name from Orders and Customer tables
    QSqlQuery query(db);
    query.prepare("SELECT c.first_name, c.last_name FROM orders o JOIN customer c ON o.customer_id = c.customer_id WHERE o.order_id = :orderId");
    query.bindValue(":orderId", orderId);
    if (query.exec() && query.next()) {
        QString firstName = query.value("first_name").toString();
        QString lastName = query.value("last_name").toString();
        ui->orderDetailsLabel->setText(QString("Customer: %1 %2 - Order ID: %3").arg(firstName, lastName, orderId));
    } else {
        ui->orderDetailsLabel->setText("Order ID: " + orderId);
        qDebug() << "Failed to load customer name:" << query.lastError().text();
    }

    // Create and set up relational model for Order_Items
    orderItemsModel = new QSqlRelationalTableModel(this, db);
    orderItemsModel->setTable("order_items");
    orderItemsModel->setRelation(2, QSqlRelation("product_details", "detail_ID", "size"));
    orderItemsModel->setFilter(QString("order_id = '%1'").arg(orderId));
    orderItemsModel->setEditStrategy(QSqlRelationalTableModel::OnManualSubmit);
    orderItemsModel->setHeaderData(0, Qt::Horizontal, "Item ID");
    orderItemsModel->setHeaderData(1, Qt::Horizontal, "Order ID");
    orderItemsModel->setHeaderData(2, Qt::Horizontal, "Product Size");
    orderItemsModel->setHeaderData(3, Qt::Horizontal, "Quantity");
    orderItemsModel->setHeaderData(4, Qt::Horizontal, "Unit Price");
    orderItemsModel->setHeaderData(5, Qt::Horizontal, "Tax Rate (%)");
    orderItemsModel->setHeaderData(6, Qt::Horizontal, "Subtotal");
    orderItemsModel->setHeaderData(7, Qt::Horizontal, "Tax Amount");
    orderItemsModel->setHeaderData(8, Qt::Horizontal, "Total");
    if (!orderItemsModel->select()) {
        qDebug() << "Order items model error:" << orderItemsModel->lastError().text();
        QMessageBox::warning(this, "Data Error", "Failed to load order items: " + orderItemsModel->lastError().text());
    }

    ui->orderDetailsTable->setModel(orderItemsModel);
    ui->orderDetailsTable->setColumnHidden(0, true); // Hide order_item_ID
    ui->orderDetailsTable->setColumnHidden(1, true); // Hide order_ID
    ui->orderDetailsTable->resizeColumnsToContents();
    ui->orderDetailsTable->horizontalHeader()->setStretchLastSection(true);
    ui->orderDetailsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->orderDetailsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Connect buttons
    connect(ui->addOrderDetailButton, &QPushButton::clicked, this, &OrderDetailsWindow::addDetail);
    connect(ui->editOrderDetailButton, &QPushButton::clicked, this, &OrderDetailsWindow::editDetail);
    connect(ui->deleteOrderDetailButton, &QPushButton::clicked, this, &OrderDetailsWindow::deleteDetail);
    connect(ui->closeWindowButton, &QPushButton::clicked, this, &QDialog::accept);
}

OrderDetailsWindow::~OrderDetailsWindow()
{
    delete orderItemsModel;
    delete ui;
}

void OrderDetailsWindow::addDetail()
{
    AddOrderDetailWindow dialog(db, this);
    if (dialog.exec() == QDialog::Accepted) {
        QSqlRecord record = orderItemsModel->record();
        record.setValue("order_id", orderId);
        record.setValue("detail_id", dialog.detailId());
        record.setValue("qty", dialog.quantity());
        record.setValue("unit_price", dialog.unitPrice());
        record.setValue("tax_rate", dialog.taxRate());
        record.setValue("sub_total", dialog.subtotal());
        record.setValue("tax_amount", dialog.taxAmount());
        record.setValue("total", dialog.total());
        if (orderItemsModel->insertRecord(-1, record)) {
            if (orderItemsModel->submitAll()) {
                ui->orderDetailsTable->resizeColumnsToContents();
            } else {
                QMessageBox::warning(this, "Error", "Failed to add order item: " + orderItemsModel->lastError().text());
            }
        } else {
            QMessageBox::warning(this, "Error", "Failed to insert order item record: " + orderItemsModel->lastError().text());
        }
    }
}

void OrderDetailsWindow::editDetail()
{
    QModelIndexList selection = ui->orderDetailsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select an order item to edit.");
        return;
    }
    int row = selection.first().row();
    QSqlRecord record = orderItemsModel->record(row);
    AddOrderDetailWindow dialog(db, this);
    dialog.setWindowTitle("Edit Order Item");
    dialog.setDetailId(record.value("detail_id").toInt());
    dialog.setQuantity(record.value("qty").toInt());
    dialog.setUnitPrice(record.value("unit_price").toDouble());
    dialog.setTaxRate(record.value("tax_rate").toDouble());
    dialog.setSubtotal(record.value("sub_total").toDouble());
    dialog.setTaxAmount(record.value("tax_amount").toDouble());
    dialog.setTotal(record.value("total").toDouble());
    if (dialog.exec() == QDialog::Accepted) {
        record.setValue("detail_id", dialog.detailId());
        record.setValue("qty", dialog.quantity());
        record.setValue("unit_price", dialog.unitPrice());
        record.setValue("tax_rate", dialog.taxRate());
        record.setValue("sub_total", dialog.subtotal());
        record.setValue("tax_amount", dialog.taxAmount());
        record.setValue("total", dialog.total());
        if (orderItemsModel->setRecord(row, record)) {
            if (orderItemsModel->submitAll()) {
                ui->orderDetailsTable->resizeColumnsToContents();
            } else {
                QMessageBox::warning(this, "Error", "Failed to save changes: " + orderItemsModel->lastError().text());
            }
        } else {
            QMessageBox::warning(this, "Error", "Failed to update order item record: " + orderItemsModel->lastError().text());
        }
    }
}

void OrderDetailsWindow::deleteDetail()
{
    QModelIndexList selection = ui->orderDetailsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select an order item to delete.");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete the selected order item?",
        QMessageBox::Yes | QMessageBox::No
        );
    if (reply == QMessageBox::Yes) {
        for (const auto& index : selection) {
            orderItemsModel->removeRow(index.row());
        }
        if (orderItemsModel->submitAll()) {
            ui->orderDetailsTable->resizeColumnsToContents();
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete order item: " + orderItemsModel->lastError().text());
        }
    }
}
