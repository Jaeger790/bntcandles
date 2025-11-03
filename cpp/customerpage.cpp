#include "../headers/customerpage.h"
#include "../ui/ui_customerpage.h"
#include "../headers/addcustomer.h"  // Fixed: incldue → include
#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlError>  // Added: For lastError().text()
#include <QSortFilterProxyModel>  // Optional: for future filtering

CustomerPage::CustomerPage(const QSqlDatabase &db, QWidget *parent)
    : QWidget(parent), ui(new Ui::CustomerPage), db(db)
{
    ui->setupUi(this);

    customerModel = new QSqlTableModel(this, db);
    customerModel->setTable("customer");
    customerModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    customerModel->setHeaderData(1, Qt::Horizontal, "First Name");
    customerModel->setHeaderData(2, Qt::Horizontal, "Last Name");
    customerModel->setHeaderData(3, Qt::Horizontal, "Phone #");
    customerModel->setHeaderData(4, Qt::Horizontal, "Email");
    customerModel->setHeaderData(5, Qt::Horizontal, "Address");
    customerModel->setHeaderData(6, Qt::Horizontal, "City");
    customerModel->setHeaderData(7, Qt::Horizontal, "State");
    customerModel->setHeaderData(8, Qt::Horizontal, "Zip Code");
    customerModel->setHeaderData(9, Qt::Horizontal, "Company");

    setupCustomerTable();
    refreshTable();

    // Connect buttons
    connect(ui->addCustomerButton, &QPushButton::clicked, this, &CustomerPage::addCustomer);
    connect(ui->editCustomerButton, &QPushButton::clicked, this, &CustomerPage::editCustomer);
    connect(ui->deleteCustomerButton, &QPushButton::clicked, this, &CustomerPage::deleteCustomer);
}

CustomerPage::~CustomerPage()
{
    delete ui;
    delete customerModel;
}

void CustomerPage::setupCustomerTable()
{
    if (!customerModel->select()) {
        QMessageBox::warning(this, "Data Error", "Failed to load customers: " + customerModel->lastError().text());
    }
    ui->customerTable->setModel(customerModel);
    ui->customerTable->setColumnHidden(0, true); // Hide customer_id
    ui->customerTable->resizeColumnsToContents();
    ui->customerTable->horizontalHeader()->setStretchLastSection(false);  // Fixed: ui-. → ui->
    ui->customerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->customerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->customerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->customerTable->viewport()->update();
}

void CustomerPage::refreshTable()  // Added: Full impl (was missing?)
{
    customerModel->select(); // Force refresh
    ui->customerTable->resizeColumnsToContents();
}

void CustomerPage::addCustomer()
{
    AddCustomerWindow dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QSqlRecord record = customerModel->record();
        record.setValue("first_name", dialog.firstName());
        record.setValue("last_name", dialog.lastName());
        record.setValue("phone", dialog.phone());
        record.setValue("email", dialog.email());
        record.setValue("address_street", dialog.address());
        record.setValue("address_city", dialog.city());
        record.setValue("address_state", dialog.state());
        record.setValue("address_zip", dialog.zip());
        record.setValue("company_name", dialog.company());
        if (customerModel->insertRecord(-1, record) && customerModel->submitAll()) {
            refreshTable();
        } else {
            QMessageBox::warning(this, "Error", "Failed to add customer: " + customerModel->lastError().text());
        }
    }
}

void CustomerPage::editCustomer()
{
    QModelIndexList selection = ui->customerTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a customer from the table to edit.");
        return;
    }
    int row = selection.first().row();
    QSqlRecord record = customerModel->record(row);
    AddCustomerWindow dialog(this);
    dialog.setWindowTitle("Edit Customer");
    dialog.setFirstName(record.value("first_name").toString());
    dialog.setLastName(record.value("last_name").toString());
    dialog.setPhone(record.value("phone").toString());
    dialog.setEmail(record.value("email").toString());
    dialog.setAddress(record.value("address_street").toString());
    dialog.setCity(record.value("address_city").toString());
    dialog.setState(record.value("address_state").toString());
    dialog.setZip(record.value("address_zip").toString());
    dialog.setCompany(record.value("company_name").toString());
    if (dialog.exec() == QDialog::Accepted) {
        record.setValue("first_name", dialog.firstName());
        record.setValue("last_name", dialog.lastName());
        record.setValue("phone", dialog.phone());
        record.setValue("email", dialog.email());
        record.setValue("address_street", dialog.address());
        record.setValue("address_city", dialog.city());
        record.setValue("address_state", dialog.state());
        record.setValue("address_zip", dialog.zip());
        record.setValue("company_name", dialog.company());
        if (customerModel->setRecord(row, record) && customerModel->submitAll()) {
            refreshTable();
        } else {
            QMessageBox::warning(this, "Error", "Failed to update customer: " + customerModel->lastError().text());
        }
    }
}

void CustomerPage::deleteCustomer()
{
    QModelIndexList selection = ui->customerTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a customer from the table to delete.");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete", "Are you sure you wish to delete the selected customer?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        std::sort(selection.begin(), selection.end(), [](const QModelIndex &a, const QModelIndex &b) {
            return a.row() > b.row();
        });
        for (const auto &index : selection) {
            customerModel->removeRow(index.row());
        }
        if (customerModel->submitAll()) {
            refreshTable();
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete customer: " + customerModel->lastError().text());
        }
    }
}