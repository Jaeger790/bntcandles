#include "addOrderDetailWindow.h"
#include "./ui_addOrderDetailWindow.h"
#include <QIntValidator>
#include <QDoubleValidator>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

AddOrderDetailWindow::AddOrderDetailWindow(const QSqlDatabase &db, QWidget *parent)
    : QDialog(parent), ui(new Ui::AddOrderDetailWindow), db(db)
{
    ui->setupUi(this);
    setWindowTitle("Add Order Item");

    // Set validators
    ui->quantityEdit->setValidator(new QIntValidator(1, 9999, this));
    ui->unitPriceEdit->setValidator(new QDoubleValidator(0.0, 999999.99, 2, this));
    ui->taxRateEdit->setValidator(new QDoubleValidator(0.0, 100.0, 2, this));
    ui->subtotalEdit->setValidator(new QDoubleValidator(0.0, 999999.99, 2, this));
    ui->taxAmountEdit->setValidator(new QDoubleValidator(0.0, 999999.99, 2, this));
    ui->totalEdit->setValidator(new QDoubleValidator(0.0, 999999.99, 2, this));

    // Populate product combo
    QSqlQuery query(db);
    query.prepare("SELECT pd.detail_ID, p.product_name, pd.size, pd.price, pd.tax_rate "
                  "FROM Product_Details pd JOIN Product p ON pd.product_ID = p.product_ID");
    if (query.exec()) {
        while (query.next()) {
            int detailId = query.value("detail_ID").toInt();
            QString productName = query.value("product_name").toString();
            QString size = query.value("size").toString();
            double price = query.value("price").toDouble();
            double taxRate = query.value("tax_rate").toDouble();
            QString displayText = QString("%1 (%2, $%3)").arg(productName, size, QString::number(price, 'f', 2));
            ui->productCombo->addItem(displayText, detailId);
        }
    } else {
        QMessageBox::warning(this, "Error", "Failed to load products: " + query.lastError().text());
    }

    // Connect product selection to update fields
    connect(ui->productCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, db](int index) {
        if (index >= 0) {
            QSqlQuery query(db);
            query.prepare("SELECT price, tax_rate FROM Product_Details WHERE detail_ID = :detailId");
            query.bindValue(":detailId", ui->productCombo->itemData(index).toInt());
            if (query.exec() && query.next()) {
                double price = query.value("price").toDouble();
                double taxRate = query.value("tax_rate").toDouble();
                ui->unitPriceEdit->setText(QString::number(price, 'f', 2));
                ui->taxRateEdit->setText(QString::number(taxRate, 'f', 2));
                // Update subtotal, tax, total when quantity changes
                int qty = ui->quantityEdit->text().toInt();
                if (qty > 0) {
                    double subtotal = qty * price;
                    double taxAmount = subtotal * (taxRate / 100.0);
                    double total = subtotal + taxAmount;
                    ui->subtotalEdit->setText(QString::number(subtotal, 'f', 2));
                    ui->taxAmountEdit->setText(QString::number(taxAmount, 'f', 2));
                    ui->totalEdit->setText(QString::number(total, 'f', 2));
                }
            }
        }
    });

    // Connect quantity change to update calculations
    connect(ui->quantityEdit, &QLineEdit::textChanged, this, [this]() {
        int qty = ui->quantityEdit->text().toInt();
        double price = ui->unitPriceEdit->text().toDouble();
        double taxRate = ui->taxRateEdit->text().toDouble();
        double subtotal = qty * price;
        double taxAmount = subtotal * (taxRate / 100.0);
        double total = subtotal + taxAmount;
        ui->subtotalEdit->setText(QString::number(subtotal, 'f', 2));
        ui->taxAmountEdit->setText(QString::number(taxAmount, 'f', 2));
        ui->totalEdit->setText(QString::number(total, 'f', 2));
    });
}

AddOrderDetailWindow::~AddOrderDetailWindow()
{
    delete ui;
}

int AddOrderDetailWindow::detailId() const
{
    return ui->productCombo->currentData().toInt();
}

int AddOrderDetailWindow::quantity() const
{
    return ui->quantityEdit->text().toInt();
}

double AddOrderDetailWindow::unitPrice() const
{
    return ui->unitPriceEdit->text().toDouble();
}

double AddOrderDetailWindow::taxRate() const
{
    return ui->taxRateEdit->text().toDouble();
}

double AddOrderDetailWindow::subtotal() const
{
    return ui->subtotalEdit->text().toDouble();
}

double AddOrderDetailWindow::taxAmount() const
{
    return ui->taxAmountEdit->text().toDouble();
}

double AddOrderDetailWindow::total() const
{
    return ui->totalEdit->text().toDouble();
}

void AddOrderDetailWindow::setDetailId(int detailId)
{
    int index = ui->productCombo->findData(detailId);
    if (index >= 0) {
        ui->productCombo->setCurrentIndex(index);
    }
}

void AddOrderDetailWindow::setQuantity(int quantity)
{
    ui->quantityEdit->setText(QString::number(quantity));
}

void AddOrderDetailWindow::setUnitPrice(double unitPrice)
{
    ui->unitPriceEdit->setText(QString::number(unitPrice, 'f', 2));
}

void AddOrderDetailWindow::setTaxRate(double taxRate)
{
    ui->taxRateEdit->setText(QString::number(taxRate, 'f', 2));
}

void AddOrderDetailWindow::setSubtotal(double subtotal)
{
    ui->subtotalEdit->setText(QString::number(subtotal, 'f', 2));
}

void AddOrderDetailWindow::setTaxAmount(double taxAmount)
{
    ui->taxAmountEdit->setText(QString::number(taxAmount, 'f', 2));
}

void AddOrderDetailWindow::setTotal(double total)
{
    ui->totalEdit->setText(QString::number(total, 'f', 2));
}
