#include "addorderwindow.h"
#include "ui_addorderwindow.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDoubleValidator>
#include <QDebug>

AddOrderWindow::AddOrderWindow(const QSqlDatabase &db, QWidget *parent)
    : QDialog(parent), ui(new Ui::AddOrderWindow), db(db)
{
    ui->setupUi(this);
    setWindowTitle("Add Order");

    // Set validators
    ui->totalAmountEdit->setValidator(new QDoubleValidator(0.0, 999999.99, 2, this));
    ui->orderDateEdit->setDate(QDate::currentDate());  // Default to today (2025-09-20 as per context)

    populateCustomers();
}

AddOrderWindow::~AddOrderWindow()
{
    delete ui;
}

void AddOrderWindow::populateCustomers()
{
    ui->customerCombo->clear();
    QSqlQuery query(db);
    query.prepare("SELECT customer_ID, first_name, last_name FROM Customer ORDER BY last_name");
    if (query.exec()) {
        while (query.next()) {
            int id = query.value("customer_ID").toInt();
            QString name = query.value("first_name").toString() + " " + query.value("last_name").toString();
            ui->customerCombo->addItem(name, id);  // Display name, store ID
        }
    } else {
        qDebug() << "Failed to load customers:" << query.lastError().text();
    }
}

int AddOrderWindow::customerId() const
{
    return ui->customerCombo->currentData().toInt();
}

QDate AddOrderWindow::orderDate() const
{
    return ui->orderDateEdit->date();
}

double AddOrderWindow::totalAmount() const
{
    return ui->totalAmountEdit->text().toDouble();
}

QString AddOrderWindow::status() const
{
    return ui->statusCombo->currentText();
}

void AddOrderWindow::setCustomerId(int customerId)
{
    int index = ui->customerCombo->findData(customerId);
    if (index != -1) ui->customerCombo->setCurrentIndex(index);
}

void AddOrderWindow::setOrderDate(const QDate &date)
{
    ui->orderDateEdit->setDate(date);
}

void AddOrderWindow::setTotalAmount(double amount)
{
    ui->totalAmountEdit->setText(QString::number(amount, 'f', 2));
}

void AddOrderWindow::setStatus(const QString &status)
{
    int index = ui->statusCombo->findText(status);
    if (index != -1) ui->statusCombo->setCurrentIndex(index);
}