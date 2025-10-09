#include "producteditor.h"
#include "ui_producteditor.h"
#include "addproductdetailswindow.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

ProductEditor::ProductEditor(const QString &productId, const QSqlDatabase &db, QWidget *parent)
    : QDialog(parent), ui(new Ui::ProductEditor), productId(productId), db(db), isNewProduct(productId.isEmpty()) {
    ui->setupUi(this);
    setWindowTitle(isNewProduct ? "Add Product" : "Edit Product");

    // Setup details model (like ProductDetailsWindow)
    detailsModel = new QSqlTableModel(this, db);
    detailsModel->setTable("product_details");
    if (!isNewProduct) {
        detailsModel->setFilter(QString("product_ID = '%1'").arg(productId));
    }
    detailsModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    detailsModel->setHeaderData(0, Qt::Horizontal, "Detail ID");
    detailsModel->setHeaderData(1, Qt::Horizontal, "Product ID");
    detailsModel->setHeaderData(2, Qt::Horizontal, "Size");
    detailsModel->setHeaderData(3, Qt::Horizontal, "Price");
    detailsModel->setHeaderData(4, Qt::Horizontal, "Tax Rate");
    detailsModel->setHeaderData(5, Qt::Horizontal, "Stock Quantity");
    if (!detailsModel->select()) {
        QMessageBox::warning(this, "Data Error", "Failed to load details: " + detailsModel->lastError().text());
    }

    ui->detailsTable->setModel(detailsModel);
    ui->detailsTable->setColumnHidden(0, true);
    ui->detailsTable->setColumnHidden(1, true);
    ui->detailsTable->resizeColumnsToContents();
    ui->detailsTable->horizontalHeader()->setStretchLastSection(true);
    ui->detailsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->detailsTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Connect buttons
    connect(ui->addDetailButton, &QPushButton::clicked, this, &ProductEditor::addDetail);
    connect(ui->editDetailButton, &QPushButton::clicked, this, &ProductEditor::editDetail);
    connect(ui->deleteDetailButton, &QPushButton::clicked, this, &ProductEditor::deleteDetail);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ProductEditor::saveChanges);
    connect(ui->saveProductButton, &QPushButton::clicked, this, &ProductEditor::saveProductName);

    if (isNewProduct) {
        // Disable detail management until product is saved
        ui->addDetailButton->setEnabled(false);
        ui->addDetailButton->setToolTip("Save the product name first");
        ui->addDetailButton->setStyleSheet("background-color: #00000079; color: #000000089;");
        ui->editDetailButton->setEnabled(false);
        ui->editDetailButton->setStyleSheet("background-color: #00000079; color: #000000089;");
        ui->editDetailButton->setToolTip("Save the product name first");
        ui->deleteDetailButton->setEnabled(false);
        ui->deleteDetailButton->setStyleSheet("background-color: #00000079; color: #000000089;");
        ui->deleteDetailButton->setToolTip("Save the product name first");
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Save)->setStyleSheet("background-color: #00000079; color: #000000089;");
        ui->buttonBox->button(QDialogButtonBox::Save)->setToolTip("Save the product name first");
    } else {
        // Hide save product button for existing products
        ui->saveProductButton->hide();
        // Load existing product name if editing
        QSqlQuery query(db);
        query.prepare("SELECT product_name FROM product WHERE product_ID = :id");
        query.bindValue(":id", productId);
        if (query.exec() && query.next()) {
            ui->productNameEdit->setText(query.value("product_name").toString());
        } else {
            QMessageBox::warning(this, "Data Error", "Failed to load product: " + query.lastError().text());
        }
    }
}

ProductEditor::~ProductEditor() {
    delete detailsModel;
    delete ui;
}

void ProductEditor::saveProductName() {
    QString name = ui->productNameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Product name cannot be empty.");
        return;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO product (product_name, stock_qty) VALUES (:name, 0)");
    query.bindValue(":name", name);
    if (!query.exec()) {
        QMessageBox::warning(this, "Database Error", "Failed to add product: " + query.lastError().text());
        return;
    }
    productId = query.lastInsertId().toString();

    // Apply filter for new details
    detailsModel->setFilter(QString("product_ID = '%1'").arg(productId));
    detailsModel->select();

    // Enable detail buttons and main Save
    ui->addDetailButton->setEnabled(true);
    ui->addDetailButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65) ; color:hsla(52, 100%, 95%, 1);");
    ui->editDetailButton->setEnabled(true);
    ui->editDetailButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65) ; color:hsla(52, 100%, 95%, 1);");
    ui->deleteDetailButton->setEnabled(true);
    ui->deleteDetailButton->setStyleSheet("background-color:hsla(319, 30%, 16%, .65) ; color:hsla(52, 100%, 95%, 1);");
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Save)->setStyleSheet("background-color:hsla(319, 30%, 16%, .65) ; color:hsla(52, 100%, 95%, 1);");

    // Disable the Save Product button after use
    ui->saveProductButton->setEnabled(false);
}

void ProductEditor::saveChanges() {
    if (productId.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please save the product name first.");
        return;
    }

    QString name = ui->productNameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Product name cannot be empty.");
        return;
    }

    QSqlQuery query(db);
    // Always update the product name (works for both new and existing)
    query.prepare("UPDATE product SET product_name = :name WHERE product_ID = :id");
    query.bindValue(":name", name);
    query.bindValue(":id", productId);
    if (!query.exec()) {
        QMessageBox::warning(this, "Database Error", "Failed to update product: " + query.lastError().text());
        return;
    }

    // Submit details changes
    if (!detailsModel->submitAll()) {
        QMessageBox::warning(this, "Database Error", "Failed to save details: " + detailsModel->lastError().text());
        return;
    }

    // Update stock_qty (if not using DB trigger)
    // updateTotalStock();

    accept();  
}


/* If Database trigger fails or not used, this function can be used to update stock_qty
void ProductEditor::updateTotalStock() {
    QSqlQuery query(db);
    query.prepare("UPDATE product SET stock_qty = (SELECT IFNULL(SUM(stock_qty), 0) FROM product_details WHERE product_ID = :id) WHERE product_ID = :id");
    query.bindValue(":id", productId);
    if (!query.exec()) {
    }
}
*/

void ProductEditor::addDetail() {
    AddProductDetailsWindow dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QSqlRecord record = detailsModel->record();
        record.setValue("product_ID", productId);
        record.setValue("size", dialog.size());
        record.setValue("price", dialog.price());
        record.setValue("tax_rate", dialog.taxRate());
        record.setValue("stock_qty", dialog.stockQuantity());
        if (detailsModel->insertRecord(-1, record) && detailsModel->submitAll()) {
            ui->detailsTable->resizeColumnsToContents();
            // updateTotalStock();  // If no DB trigger
        } else {
            QMessageBox::warning(this, "Error", "Failed to add detail: " + detailsModel->lastError().text());
        }
    }
}

void ProductEditor::editDetail() {
    QModelIndexList selection = ui->detailsTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a product detail to edit.");
        return;
    }
    int row = selection.first().row();
    QSqlRecord record = detailsModel->record(row);
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
        if (detailsModel->setRecord(row, record) && detailsModel->submitAll()) {
            ui->detailsTable->resizeColumnsToContents();
            // updateTotalStock();  // If no DB trigger
        } else {
            QMessageBox::warning(this, "Error", "Failed to update product detail: " + detailsModel->lastError().text());
        }
    }
}

void ProductEditor::deleteDetail() {
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
            detailsModel->removeRow(index.row());
        }
        if (detailsModel->submitAll()) {
            ui->detailsTable->resizeColumnsToContents();
            // updateTotalStock();  // If no DB trigger
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete product detail: " + detailsModel->lastError().text());
        }
    }
}