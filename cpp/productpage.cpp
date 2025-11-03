#include "../headers/productpage.h"
#include "../ui/ui_productpage.h"
#include "../headers/producteditor.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>  // Added: For lastError().text()
#include <algorithm>  // For std::sort

ProductPage::ProductPage(const QSqlDatabase &db, QWidget *parent)
    : QWidget(parent), ui(new Ui::ProductPage), db(db)
{
    ui->setupUi(this);

    productModel = new QSqlTableModel(this, db);
    productModel->setTable("product");
    productModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    // Fix original bug: separate headers for name, desc, stock (assuming col 3 = stock)
    productModel->setHeaderData(1, Qt::Horizontal, "Product Name");
    productModel->setHeaderData(2, Qt::Horizontal, "Description");
    productModel->setHeaderData(3, Qt::Horizontal, "Total Stock");

    setupProductTable();
    refreshTable();

    // Connect buttons
    connect(ui->addProductButton, &QPushButton::clicked, this, &ProductPage::addProduct);
    connect(ui->editProductButton, &QPushButton::clicked, this, &ProductPage::editProduct);
    connect(ui->deleteProductButton, &QPushButton::clicked, this, &ProductPage::deleteProduct);

    // Double-click to edit (Fixed: capture &db)
    connect(ui->productTable, &QTableView::doubleClicked, this, [this, &db](const QModelIndex &index) {
        ui->productTable->selectRow(index.row());
        QString productId = productModel->data(productModel->index(index.row(), 0)).toString();
        ProductEditor editor(productId, db, this);
        if (editor.exec() == QDialog::Accepted) {
            refreshTable();
        }
    });
}

ProductPage::~ProductPage()
{
    delete ui;
    delete productModel;
}

void ProductPage::setupProductTable()
{
    if (!productModel->select()) {
        QMessageBox::warning(this, "Data Error", "Failed to load products: " + productModel->lastError().text());
        return;
    }
    ui->productTable->setModel(productModel);
    ui->productTable->setColumnHidden(0, true); // Hide product_id
    ui->productTable->resizeColumnsToContents();
    ui->productTable->horizontalHeader()->setStretchLastSection(false);
    ui->productTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->productTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->productTable->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void ProductPage::refreshTable()
{
    productModel->select(); // Force refresh
    ui->productTable->resizeColumnsToContents();
}

void ProductPage::addProduct()
{
    ProductEditor editor("", db, this); // Empty ID for add
    if (editor.exec() == QDialog::Accepted) {
        refreshTable();
    }
}

void ProductPage::editProduct()
{
    QModelIndexList selection = ui->productTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a product to edit.");
        return;
    }
    int row = selection.first().row();
    QString productId = productModel->data(productModel->index(row, 0)).toString();
    ProductEditor editor(productId, db, this);
    if (editor.exec() == QDialog::Accepted) {
        refreshTable();
    }
}

void ProductPage::deleteProduct()
{
    QModelIndexList selection = ui->productTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a product to delete.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete the selected product(s) and all associated details?",
        QMessageBox::Yes | QMessageBox::No
    );
    if (reply != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query(db);
    bool success = true;

    // Sort descending to avoid index shifts
    std::sort(selection.begin(), selection.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() > b.row();
    });

    for (const auto &index : selection) {
        QString productId = productModel->data(productModel->index(index.row(), 0)).toString();

        // Check for dependencies in order_items
        query.prepare("SELECT COUNT(*) FROM product_details pd "
                      "JOIN order_items oi ON pd.detail_ID = oi.detail_ID "
                      "WHERE pd.product_ID = :productId");
        query.bindValue(":productId", productId);
        if (!query.exec()) {
            QMessageBox::warning(this, "Error", "Failed to check dependencies: " + query.lastError().text());
            success = false;
            break;
        }
        query.next();
        if (query.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Error", "Cannot delete product: Associated details are referenced in Order_Items.");
            success = false;
            break;  // Or continue? For multi-select, maybe skip and proceed with others
        }

        // Delete product_details records
        query.prepare("DELETE FROM product_details WHERE product_ID = :productId");
        query.bindValue(":productId", productId);
        if (!query.exec()) {
            QMessageBox::warning(this, "Error", "Failed to delete product details: " + query.lastError().text());
            success = false;
            break;
        }

        // Delete product record
        if (!productModel->removeRow(index.row())) {
            QMessageBox::warning(this, "Error", "Failed to remove product record.");
            success = false;
            break;
        }
    }

    if (success && productModel->submitAll()) {
        refreshTable();
    } else if (success) {
        QMessageBox::warning(this, "Error", "Failed to submit changes: " + productModel->lastError().text());
    }
}