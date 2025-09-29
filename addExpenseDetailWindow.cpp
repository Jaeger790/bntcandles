#include "addExpenseDetailWindow.h"
#include <QDialog>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include "./ui_addExpenseDetailWindow.h"

AddExpenseDetailWindow::AddExpenseDetailWindow(QWidget *parent): QDialog(parent), ui(new Ui::AddExpenseDetailWindow)
{
    ui->setupUi(this);
    setWindowTitle("Add Expense Detail");

    // Set validators for numeric fields
    ui->quantityEdit->setValidator(new QIntValidator(0, 9999, this));
    ui->subtotalEdit->setValidator(new QDoubleValidator(0.0, 999999.99, 2, this));
    ui->taxEdit->setValidator(new QDoubleValidator(0.0, 999999.99, 2, this));
    ui->shippingEdit->setValidator(new QDoubleValidator(0.0, 999999.99, 2, this));
    ui->promotionEdit->setValidator(new QDoubleValidator(0.0, 999999.99, 2, this));
    ui->taxRateEdit->setValidator(new QDoubleValidator(0.0, 100.0, 4, this));

    QFile taxRateFile(":/taxRates.txt");
    if (taxRateFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&taxRateFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                ui->taxRateEdit->addItem(line);
            }
        }
        taxRateFile.close();
    } else {
        qWarning("Could not open taxRates.txt");
    }
}

AddExpenseDetailWindow::~AddExpenseDetailWindow()
{
    delete ui;
}

QString AddExpenseDetailWindow::date() const
{
    return ui->dateEdit->date().toString(Qt::ISODate);
}

QString AddExpenseDetailWindow::description() const
{
    return ui->descriptionEdit->text();
}

QString AddExpenseDetailWindow::category() const
{
    return ui->categoryEdit->text();
}

QString AddExpenseDetailWindow::paymentMethod() const
{
    return ui->paymentMethodEdit->text();
}

QString AddExpenseDetailWindow::source() const
{
    return ui->sourceEdit->text();
}

QString AddExpenseDetailWindow::itemName() const
{
    return ui->itemNameEdit->text();
}

int AddExpenseDetailWindow::quantity() const
{
    return ui->quantityEdit->text().toInt();
}

double AddExpenseDetailWindow::itemSubtotal() const
{
    return ui->subtotalEdit->text().toDouble();
}

double AddExpenseDetailWindow::itemTax() const
{
    return ui->taxEdit->text().toDouble();
}

double AddExpenseDetailWindow::itemShipping() const
{
    return ui->shippingEdit->text().toDouble();
}

double AddExpenseDetailWindow::itemPromotion() const
{
    return ui->promotionEdit->text().toDouble();
}

double AddExpenseDetailWindow::itemTaxRate() const
{
    return ui->taxRateEdit->currentText().toDouble();
}

QString AddExpenseDetailWindow::notes() const
{
    return ui->notesEdit->toPlainText();
}

void AddExpenseDetailWindow::setDate(const QString &date)
{
    QDate qdate = QDate::fromString(date, Qt::ISODate); // Convert QString to QDate
    if (qdate.isValid()) {
        ui->dateEdit->setDate(qdate);
    } else {
        ui->dateEdit->setDate(QDate::currentDate()); // Fallback to current date
    }
}

void AddExpenseDetailWindow::setDescription(const QString &description)
{
    ui->descriptionEdit->setText(description);
}

void AddExpenseDetailWindow::setCategory(const QString &category)
{
    ui->categoryEdit->setText(category);
}

void AddExpenseDetailWindow::setPaymentMethod(const QString &paymentMethod)
{
    ui->paymentMethodEdit->setText(paymentMethod);
}

void AddExpenseDetailWindow::setSource(const QString &source)
{
    ui->sourceEdit->setText(source);
}

void AddExpenseDetailWindow::setItemName(const QString &itemName)
{
    ui->itemNameEdit->setText(itemName);
}

void AddExpenseDetailWindow::setQuantity(int quantity)
{
    ui->quantityEdit->setText(QString::number(quantity));
}

void AddExpenseDetailWindow::setItemSubtotal(double subtotal)
{
    ui->subtotalEdit->setText(QString::number(subtotal, 'f', 2));
}

void AddExpenseDetailWindow::setItemTax(double tax)
{
    ui->taxEdit->setText(QString::number(tax, 'f', 2));
}

void AddExpenseDetailWindow::setItemShipping(double shipping)
{
    ui->shippingEdit->setText(QString::number(shipping, 'f', 2));
}

void AddExpenseDetailWindow::setItemPromotion(double promotion)
{
    ui->promotionEdit->setText(QString::number(promotion, 'f', 2));
}

void AddExpenseDetailWindow::setItemTaxRate(double taxRate)
{
    int index = ui->taxRateEdit->findData(taxRate,Qt::MatchExactly);
    if (index != -1){
        ui->taxRateEdit->setCurrentIndex(index);
    }else{
        ui->taxRateEdit->setCurrentIndex(-1);
    }
}

void AddExpenseDetailWindow::setNotes(const QString &notes)
{
    ui->notesEdit->setPlainText(notes);
}
