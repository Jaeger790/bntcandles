#include "../headers/addcustomer.h"
#include "../ui/ui_addcustomer.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

AddCustomerWindow::AddCustomerWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddCustomerWindow)
{
    ui->setupUi(this);
    setWindowTitle("Add Customer");
    // Load states from resources
    QFile statesFile(":/states");
    if (statesFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&statesFile);
        while (!stream.atEnd()) {
            QString state = stream.readLine().trimmed();
            if (!state.isEmpty()) {
                ui->stateComboBox->addItem(state);
            }
        }
        statesFile.close();
        ui->stateComboBox->setCurrentIndex(-1); // No default selection
    } else {
        qDebug() << "Failed to load states from :/states.txt";
    }
}


AddCustomerWindow::~AddCustomerWindow()
{
    delete ui;
}

QString AddCustomerWindow::firstName() const{
    return ui->firstNameTextbox->text();
}
QString AddCustomerWindow::lastName() const{
    return ui->lastNameTextbox->text();
}
QString AddCustomerWindow::phone() const
{
    return ui->phoneTextbox->text();
}

QString AddCustomerWindow::email() const
{
    return ui->emailTextbox->text();
}

QString AddCustomerWindow::address() const
{
    return ui->addressTextbox->text();
}

QString AddCustomerWindow::city() const
{
    return ui->cityTextbox->text();
}

QString AddCustomerWindow::state() const
{
    return ui->stateComboBox->currentText();
}

QString AddCustomerWindow::zip() const
{
    return ui->zipTextbox->text();
}

QString AddCustomerWindow::company() const{
    return ui->companyTextbox->text();
}

void AddCustomerWindow::setFirstName(const QString &firstName)
{
    ui->firstNameTextbox->setText(firstName);
}

void AddCustomerWindow::setLastName(const QString &lastName)
{
    ui->lastNameTextbox->setText(lastName);
}

void AddCustomerWindow::setPhone(const QString &phone)
{
    ui->phoneTextbox->setText(phone);
}

void AddCustomerWindow::setEmail(const QString &email)
{
    ui->emailTextbox->setText(email);
}

void AddCustomerWindow::setAddress(const QString &address)
{
    ui->addressTextbox->setText(address);
}

void AddCustomerWindow::setCity(const QString &city)
{
    ui->cityTextbox->setText(city);
}

void AddCustomerWindow::setState(const QString &state){
    int index = ui->stateComboBox->findText(state,Qt::MatchExactly);
    if(index != -1){
        ui->stateComboBox->setCurrentIndex(index);
    }else{
        ui->stateComboBox->setCurrentIndex(-1);
    }
}

void AddCustomerWindow::setZip(const QString &zip)
{
    ui->cityTextbox->setText(zip);
}

void AddCustomerWindow::setCompany(const QString &company){
    ui->companyTextbox->setText(company);
}
