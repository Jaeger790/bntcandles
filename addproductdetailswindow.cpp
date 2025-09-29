#include "addproductdetailswindow.h"
#include "ui_addproductdetailswindow.h"
#include <QDoubleValidator>
#include <QIntValidator>

AddProductDetailsWindow::AddProductDetailsWindow(QWidget *parent) : QDialog(parent), ui(new Ui::AddProductDetailsWindow) {
    ui->setupUi(this);
    setWindowTitle("Add Product Detail");

    // Set validators
    ui->priceEdit->setValidator(new QDoubleValidator(0.0, 999999.99, 2, this));
    ui->taxRateEdit->setValidator(new QDoubleValidator(0.0, 100.0, 3, this));
    ui->stockQtyEdit->setMinimum(0);
}

AddProductDetailsWindow::~AddProductDetailsWindow() {
    delete ui;
}

QString AddProductDetailsWindow::size() const {
    return ui->sizeEdit->text();
}

double AddProductDetailsWindow::price() const {
    return ui->priceEdit->text().toDouble();
}

double AddProductDetailsWindow::taxRate() const {
    return ui->taxRateEdit->text().toDouble();
}

int AddProductDetailsWindow::stockQuantity() const {
    return ui->stockQtyEdit->value();
}

void AddProductDetailsWindow::setSize(const QString &size) {
    ui->sizeEdit->setText(size);
}

void AddProductDetailsWindow::setPrice(double price) {
    ui->priceEdit->setText(QString::number(price, 'f', 2));
}

void AddProductDetailsWindow::setTaxRate(double taxRate) {
    ui->taxRateEdit->setText(QString::number(taxRate, 'f', 3));
}

void AddProductDetailsWindow::setStockQuantity(int stockQty) {
    ui->stockQtyEdit->setValue(stockQty);
}
