#include "productdetailswindow.h"
#include "ui_productdetailswindow.h"
#include "addproductdetailswindow.h"
#include <QSqlRecord>
#include <QSqlQuery>  // Added for querying product name
#include <QSqlError>  // Added for error handling
#include <QMessageBox>
#include <QDebug>

ProductDetailsWindow::ProductDetailsWindow(const QString &productId, const QSqlDatabase &db, QWidget *parent)
    : QDialog(parent), ui(new Ui::ProductDetailsWindow), productId(productId), db(db) {
    ui->setupUi(this);

    // Set product ID label
    // Query the product name from the Product table
    QString productName;
    QSqlQuery query(db);
    query.prepare("SELECT product_name FROM product WHERE product_ID = :id");
    query.bindValue(":id", productId);
    if (query.exec() && query.next()) {
        productName = query.value("product_name").toString();
    } else {
        productName = "Unknown Product";
        qDebug() << "Failed to fetch product name for ID" << productId << ":" << query.lastError().text();
    }
    ui->productIdLabel->setText(QString("%1").arg(productName ));

    // Create and set up model for Product_Details
    productDetailsModel = new QSqlTableModel(this, db);
    productDetailsModel->setTable("product_details");
    productDetailsModel->setFilter(QString("product_ID = '%1'").arg(productId));
    productDetailsModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    productDetailsModel->setHeaderData(0, Qt::Horizontal, "Detail ID");
    productDetailsModel->setHeaderData(1, Qt::Horizontal, "Product ID");
    productDetailsModel->setHeaderData(2, Qt::Horizontal, "Size");
    productDetailsModel->setHeaderData(3, Qt::Horizontal, "Price");
    productDetailsModel->setHeaderData(4, Qt::Horizontal, "Tax Rate");
    productDetailsModel->setHeaderData(5, Qt::Horizontal, "Stock Quantity");
    if (!productDetailsModel->select()) {
        qDebug() << "Product details model error:" << productDetailsModel->lastError().text();
        QMessageBox::warning(this, "Data Error", "Failed to load product details: " + productDetailsModel->lastError().text());
    }

    ui->detailsTable->setModel(productDetailsModel);
    ui->detailsTable->setColumnHidden(0, true); // Hide detail_ID
    ui->detailsTable->setColumnHidden(1, true); // Hide product_ID
    ui->detailsTable->resizeColumnsToContents();
    ui->detailsTable->horizontalHeader()->setStretchLastSection(true);
    ui->detailsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->detailsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Connect buttons
    connect(ui->addDetailButton, &QPushButton::clicked, this, &ProductDetailsWindow::addDetail);
    connect(ui->editDetailButton, &QPushButton::clicked, this, &ProductDetailsWindow::editDetail);
    connect(ui->deleteDetailButton, &QPushButton::clicked, this, &ProductDetailsWindow::deleteDetail);
}

ProductDetailsWindow::~ProductDetailsWindow() {
    delete productDetailsModel;
    delete ui;
}

void ProductDetailsWindow::addDetail() {
    AddProductDetailsWindow dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QSqlRecord record = productDetailsModel->record();
        record.setValue("product_ID", productId);
        record.setValue("size", dialog.size());
        record.setValue("price", dialog.price());
        record.setValue("tax_rate", dialog.taxRate());
        record.setValue("stock_qty", dialog.stockQuantity());
        if (productDetailsModel->insertRecord(-1, record)) {
            if (productDetailsModel->submitAll()) {
                ui->detailsTable->resizeColumnsToContents();
            } else {
                QMessageBox::warning(this, "Error", "Failed to add product detail: " + productDetailsModel->lastError().text());
            }
        } else {
            QMessageBox::warning(this, "Error", "Failed to insert product detail record: " + productDetailsModel->lastError().text());
        }
    }
}

void ProductDetailsWindow::editDetail() {
    QModelIndexList selection = ui->detailsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a product detail to edit.");
        return;
    }
    int row = selection.first().row();
    QSqlRecord record = productDetailsModel->record(row);
    AddProductDetailsWindow dialog(this);
    dialog.setWindowTitle("Edit Product Detail");
    dialog.setSize(record.value("size").toString());
    dialog.setPrice(record.value("price").toDouble());
    dialog.setTaxRate(record.value("tax_rate").toDouble());
    dialog.setStockQuantity(record.value("stock_qty").toInt());
    if (dialog.exec() == QDialog::Accepted) {
        record.setValue("size", dialog.size());
        record.setValue("price", dialog.price());
        record.setValue("tax_rate", dialog.taxRate());
        record.setValue("stock_qty", dialog.stockQuantity());
        if (productDetailsModel->setRecord(row, record)) {
            if (productDetailsModel->submitAll()) {
                ui->detailsTable->resizeColumnsToContents();
            } else {
                QMessageBox::warning(this, "Error", "Failed to update product detail: " + productDetailsModel->lastError().text());
            }
        } else {
            QMessageBox::warning(this, "Error", "Failed to set product detail record: " + productDetailsModel->lastError().text());
        }
    }
}

void ProductDetailsWindow::deleteDetail() {
    QModelIndexList selection = ui->detailsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a product detail to delete.");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete the selected product detail?",
        QMessageBox::Yes | QMessageBox::No
        );
    if (reply == QMessageBox::Yes) {
        for (const auto& index : selection) {
            productDetailsModel->removeRow(index.row());
        }
        if (productDetailsModel->submitAll()) {
            ui->detailsTable->resizeColumnsToContents();
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete product detail: " + productDetailsModel->lastError().text());
        }
    }
}

