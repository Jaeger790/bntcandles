// addExpenseDetailWindow.cpp
#include "../headers/addExpenseDetailWindow.h"
#include "../ui/ui_addexpensedetailwindow.h"  // From .ui
#include <QDate>
#include <QMessageBox>
#include <QDoubleValidator>  // For price/qty input
#include <QIntValidator>

AddExpenseDetailWindow::AddExpenseDetailWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::AddExpenseDetailWindow)
{
    ui->setupUi(this);
    setWindowTitle("Add Manual Expense Detail");

    // Updated: Trimmed tax rates to your spec (7.25%, 8.5%); editable for custom
    ui->taxRateComboBox->addItems({"7.25", "8.5"});
    ui->taxRateComboBox->setCurrentText("7.25");  // Default to first option
    ui->taxRateComboBox->setEditable(true);  // Allow overrides (e.g., 7.75)

    // Combos (category unchanged)
    ui->categoryComboBox->addItems({"Supplies", "Equipment", "Marketing", "Travel", "Meals", "Other"});

    // Validators for ease
    ui->quantityEdit->setValidator(new QIntValidator(1, 999, this));
    ui->unitPriceEdit->setValidator(new QDoubleValidator(0.00, 9999.99, 2, this));

    ui->dateEdit->setDate(QDate::currentDate());
    ui->quantityEdit->setText("1");
}

AddExpenseDetailWindow::~AddExpenseDetailWindow()
{
    delete ui;
}

// Getters (with calcs)
QString AddExpenseDetailWindow::date() const { return ui->dateEdit->date().toString(Qt::ISODate); }
QString AddExpenseDetailWindow::description() const { return ui->descriptionEdit->text().trimmed(); }
QString AddExpenseDetailWindow::category() const { return ui->categoryComboBox->currentText(); }
QString AddExpenseDetailWindow::merchant() const { return ui->merchantEdit->text().trimmed(); }
QString AddExpenseDetailWindow::itemName() const { return ui->itemNameEdit->text().trimmed(); }
int AddExpenseDetailWindow::quantity() const {
    bool ok;
    int q = ui->quantityEdit->text().toInt(&ok);
    return ok ? q : 1;
}
double AddExpenseDetailWindow::unitPrice() const {
    bool ok;
    double p = ui->unitPriceEdit->text().toDouble(&ok);
    return ok ? p : 0.0;
}
double AddExpenseDetailWindow::itemTaxRate() const {  // Returns % (e.g., 7.25)
    bool ok;
    double rate = ui->taxRateComboBox->currentText().toDouble(&ok);
    return ok ? rate : 7.25;  // Fallback to default
}
double AddExpenseDetailWindow::itemSubtotal() const { return quantity() * unitPrice(); }
double AddExpenseDetailWindow::itemTax() const { return itemSubtotal() * (itemTaxRate() / 100.0); }
double AddExpenseDetailWindow::itemShipping() const { return 0.0; }
double AddExpenseDetailWindow::itemPromotion() const { return 0.0; }
QString AddExpenseDetailWindow::notes() const { return ui->notesEdit->toPlainText().trimmed(); }

// Setters
void AddExpenseDetailWindow::setDate(const QString &dateStr) { ui->dateEdit->setDate(QDate::fromString(dateStr, Qt::ISODate)); }
void AddExpenseDetailWindow::setDescription(const QString &description) { ui->descriptionEdit->setText(description); }
void AddExpenseDetailWindow::setCategory(const QString &category) { ui->categoryComboBox->setCurrentText(category); }
void AddExpenseDetailWindow::setMerchant(const QString &merchant) { ui->merchantEdit->setText(merchant); }
void AddExpenseDetailWindow::setItemName(const QString &itemName) { ui->itemNameEdit->setText(itemName); }
void AddExpenseDetailWindow::setQuantity(int quantity) { ui->quantityEdit->setText(quantity > 0 ? QString::number(quantity) : "1"); }
void AddExpenseDetailWindow::setUnitPrice(double price) { ui->unitPriceEdit->setText(price >= 0 ? QString::number(price, 'f', 2) : "0.00"); }
void AddExpenseDetailWindow::setItemTaxRate(double taxRate) {  // As %
    QString rateStr = QString::number(taxRate, 'f', 2);
    ui->taxRateComboBox->setCurrentText(rateStr);
}
void AddExpenseDetailWindow::setNotes(const QString &notes) { ui->notesEdit->setPlainText(notes); }