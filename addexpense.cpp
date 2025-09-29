#include "addexpense.h"
#include "ui_addexpense.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QSqlError>
#include <QPushButton>

AddExpense::AddExpense(QSqlDatabase &db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddExpense),
    m_db(db)
{
    ui->setupUi(this);

    // Set default values
    ui->dateEdit->setDate(QDate::currentDate());
    ui->paymentMethodCombo->setCurrentIndex(0);
    ui->sourceEdit->setText("");
    ui->descriptionEdit->setText("");
    ui->categoryCombo->setCurrentIndex(0);
    ui->itemNameEdit->setText("");
    ui->quantitySpinBox->setValue(1);
    ui->itemSubtotalSpinBox->setValue(0.0);
    ui->itemTaxSpinBox->setValue(0.0);
    ui->itemShippingSpinBox->setValue(0.0);
    ui->itemPromotionSpinBox->setValue(0.0);
    ui->itemTaxRateSpinBox->setValue(0.0);
    ui->notesEdit->setPlainText("");

    // Connect buttons
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AddExpense::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &AddExpense::reject);
}

AddExpense::~AddExpense()
{
    delete ui;
}

void AddExpense::setDate(const QDate &date)
{
    ui->dateEdit->setDate(date);
}

void AddExpense::setPaymentMethod(const QString &paymentMethod)
{
    int index = ui->paymentMethodCombo->findText(paymentMethod);
    if (index != -1) {
        ui->paymentMethodCombo->setCurrentIndex(index);
    }
}

void AddExpense::setSource(const QString &source)
{
    ui->sourceEdit->setText(source);
}

void AddExpense::setDescription(const QString &description)
{
    ui->descriptionEdit->setText(description);
}

void AddExpense::setCategory(const QString &category)
{
    int index = ui->categoryCombo->findText(category);
    if (index != -1) {
        ui->categoryCombo->setCurrentIndex(index);
    }
}

void AddExpense::setItemName(const QString &itemName)
{
    ui->itemNameEdit->setText(itemName);
}

void AddExpense::setQuantity(int quantity)
{
    ui->quantitySpinBox->setValue(quantity);
}

void AddExpense::setItemSubtotal(double subtotal)
{
    ui->itemSubtotalSpinBox->setValue(subtotal);
}

void AddExpense::setItemTax(double tax)
{
    ui->itemTaxSpinBox->setValue(tax);
}

void AddExpense::setItemShipping(double shipping)
{
    ui->itemShippingSpinBox->setValue(shipping);
}

void AddExpense::setItemPromotion(double promotion)
{
    ui->itemPromotionSpinBox->setValue(promotion);
}

void AddExpense::setItemTaxRate(double taxRate)
{
    ui->itemTaxRateSpinBox->setValue(taxRate);
}

void AddExpense::setNotes(const QString &notes)
{
    ui->notesEdit->setPlainText(notes);
}

void AddExpense::setOrderIdDisplay(const QString &orderId) // Ensure this is present
{
    ui->orderIdDisplay->setText(orderId);
}

QDate AddExpense::date() const
{
    return ui->dateEdit->date();
}

QString AddExpense::paymentMethod() const
{
    return ui->paymentMethodCombo->currentText();
}

QString AddExpense::source() const
{
    return ui->sourceEdit->text();
}

QString AddExpense::description() const
{
    return ui->descriptionEdit->text();
}

QString AddExpense::category() const
{
    return ui->categoryCombo->currentText();
}

QString AddExpense::itemName() const
{
    return ui->itemNameEdit->text();
}

int AddExpense::quantity() const
{
    return ui->quantitySpinBox->value();
}

double AddExpense::itemSubtotal() const
{
    return ui->itemSubtotalSpinBox->value();
}

double AddExpense::itemTax() const
{
    return ui->itemTaxSpinBox->value();
}

double AddExpense::itemShipping() const
{
    return ui->itemShippingSpinBox->value();
}

double AddExpense::itemPromotion() const
{
    return ui->itemPromotionSpinBox->value();
}

double AddExpense::itemTaxRate() const
{
    return ui->itemTaxRateSpinBox->value();
}

QString AddExpense::notes() const
{
    return ui->notesEdit->toPlainText();
}

void AddExpense::accept()
{
    if (description().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Description is required.");
        return;
    }
    if (itemName().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Item Name is required.");
        return;
    }
    if (quantity() < 1) {
        QMessageBox::warning(this, "Input Error", "Quantity must be at least 1.");
        return;
    }
    if (itemSubtotal() < 0) {
        QMessageBox::warning(this, "Input Error", "Subtotal must be non-negative.");
        return;
    }
    if (itemTax() < 0) {
        QMessageBox::warning(this, "Input Error", "Tax must be non-negative.");
        return;
    }
    if (itemShipping() < 0) {
        QMessageBox::warning(this, "Input Error", "Shipping must be non-negative.");
        return;
    }
    if (itemPromotion() < 0) {
        QMessageBox::warning(this, "Input Error", "Promotion must be non-negative.");
        return;
    }
    if (itemTaxRate() < 0 || itemTaxRate() > 100) {
        QMessageBox::warning(this, "Input Error", "Tax rate must be between 0 and 100.");
        return;
    }

    QDialog::accept();
}
