#include "../headers/addorderdetailwindow.h"
#include "../ui/ui_addorderdetailwindow.h"
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

    // Set validators (only for remaining fields)
    ui->quantityEdit->setValidator(new QIntValidator(1, 9999, this));

    // Populate product combo
    QSqlQuery query(db);
    query.prepare("SELECT pd.detail_ID, p.product_name, pd.size, pd.price, pd.tax_rate "
                  "FROM product_details pd JOIN product p ON pd.product_ID = p.product_ID");
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
        ui->productCombo->setCurrentIndex(-1);
    } else {
        QMessageBox::warning(this, "Error", "Failed to load products: " + query.lastError().text());
    }

    // Connect product selection to update fields
    connect(ui->productCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, db](int index) {
        if (index >= 0) {
            // Load into members (no UI sets needed)
            m_detailId = ui->productCombo->itemData(index).toInt();
            QSqlQuery query(db);
            query.prepare("SELECT price, tax_rate FROM product_details WHERE detail_ID = :detailId");
            query.bindValue(":detailId", m_detailId);
            if (query.exec() && query.next()) {
                m_unitPrice = query.value("price").toDouble();
                m_taxRate = query.value("tax_rate").toDouble();
                loadHTML();
                // Update totals when quantity changes (if qty already set)
                int qty = ui->quantityEdit->text().toInt();
                if (qty > 0) {
                    m_subtotal = qty * m_unitPrice;
                    m_taxAmount = m_subtotal * m_taxRate;
                    m_total = m_subtotal + m_taxAmount;
                    loadHTML();  // Refresh display
                }
            }
        }
    });

    // Connect quantity change to update calculations
    connect(ui->quantityEdit, &QLineEdit::textChanged, this, [this]() {
        int qty = ui->quantityEdit->text().toInt();
        m_subtotal = qty * m_unitPrice;
        m_taxAmount = m_subtotal * m_taxRate;
        m_total = m_subtotal + m_taxAmount;
        loadHTML();
    });

    loadHTML();
}

AddOrderDetailWindow::~AddOrderDetailWindow()
{
    delete ui;
}

void AddOrderDetailWindow::loadHTML(){
    // Enhanced HTML: Include qty/product for context, $ for money, % for tax
    QString html = R"(
        <html>
        <head>
            <style>
                body { font-family: Arial, sans-serif; margin: 1px; background-color: rgba(45, 49, 67, 1);  }
                h2 { color: #fff; font-size:18px; }
                p { color: white; font-size:14px; }
            </style>
        </head>
        <html>
            <body>
                
                <h2>Product</h2>
                    <p>%6</p>
                <h2>Quantity</h2>
                    <p>%7</p>
                <h2>Unit Price</h2>
                    <p>$%1</p>
                <h2>Tax Rate</h2>
                    <p>%2%</p>
                <h2>Subtotal</h2>
                    <p>$%3</p>
                <h2>Tax Amount</h2>
                    <p>$%4</p>
                <h2>Total</h2>
                    <p>$%5</p>
                
            </body>
        </html>
    )";
    // Args: unitPrice, taxRate*100, subtotal, taxAmount, total, productName, quantity
    QString productName = ui->productCombo->currentText().isEmpty() ? "None" : ui->productCombo->currentText();
    int qty = ui->quantityEdit->text().toInt();
    html = html.arg(QString::number(unitPrice(), 'f', 2))
                .arg(taxRate() * 100, 0, 'f', 2)
                .arg(QString::number(subtotal(), 'f', 2))
                .arg(QString::number(taxAmount(), 'f', 2))
                .arg(QString::number(total(), 'f', 2))
                .arg(productName)
                .arg(qty);

    ui->textBrowser->setHtml(html);
} 

int AddOrderDetailWindow::detailId() const
{
    return m_detailId;  // Now from member
}

int AddOrderDetailWindow::quantity() const
{
    return ui->quantityEdit->text().toInt();
}

double AddOrderDetailWindow::unitPrice() const
{
    return m_unitPrice;
}

double AddOrderDetailWindow::taxRate() const
{
    return m_taxRate;
}

double AddOrderDetailWindow::subtotal() const
{
    return m_subtotal;
}

double AddOrderDetailWindow::taxAmount() const
{
    return m_taxAmount;
}

double AddOrderDetailWindow::total() const
{
    return m_total;
}

void AddOrderDetailWindow::setDetailId(int detailId)
{
    m_detailId = detailId;
    int index = ui->productCombo->findData(detailId);
    if (index >= 0) {
        ui->productCombo->setCurrentIndex(index);  // Triggers connect to load price/tax
    }
    loadHTML();
}

void AddOrderDetailWindow::setQuantity(int quantity)
{
    ui->quantityEdit->setText(QString::number(quantity));  // Triggers textChanged
}

void AddOrderDetailWindow::setUnitPrice(double unitPrice)
{
    m_unitPrice = unitPrice;
    loadHTML();
}

void AddOrderDetailWindow::setTaxRate(double taxRate)
{
    m_taxRate = taxRate;
    loadHTML();
}

void AddOrderDetailWindow::setSubtotal(double subtotal)
{
    m_subtotal = subtotal;
    loadHTML();
}

void AddOrderDetailWindow::setTaxAmount(double taxAmount)
{
    m_taxAmount = taxAmount;
    loadHTML();
}

void AddOrderDetailWindow::setTotal(double total)
{
    m_total = total;
    loadHTML();
}