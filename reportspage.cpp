#include "reportspage.h"
#include "ui_reportspage.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ReportsPage::ReportsPage(const QSqlDatabase &db, QWidget *parent)
    : QWidget(parent), ui(new Ui::ReportsPage), db(db)
{
    qDebug() << "ReportsPage: Database open:" << db.isOpen();
    ui->setupUi(this);

    // Set up models
    customerSpendingModel = new QSqlQueryModel(this);
    ui->customerSpendingTable->setModel(customerSpendingModel);
    ui->customerSpendingTable->horizontalHeader()->setStretchLastSection(false);

    candleSalesModel = new QSqlQueryModel(this);
    ui->candleSalesTable->setModel(candleSalesModel);
    ui->candleSalesTable->horizontalHeader()->setStretchLastSection(false);

    refreshReports();
}

ReportsPage::~ReportsPage()
{
    delete ui;
}

void ReportsPage::refreshReports()
{
    updateTotals();
    updateCustomerSpending();
    updateCandleSales();
}

void ReportsPage::updateTotals()
{
    QSqlQuery query(db);

    // Total Income from Orders
    query.exec("SELECT SUM(grand_total) AS total_income FROM orders WHERE status = 'Paid'");
    if (!query.isActive()) {
        qDebug() << "Total income query error:" << query.lastError().text();
    }
    double totalIncome = 0.0;
    if (query.next()) {
        QVariant value = query.value("total_income");
        if (!value.isNull()) totalIncome = value.toDouble();
    }
    ui->totalIncomeLabel->setText(QString("Total Income: $%1").arg(totalIncome, 0, 'f', 2));

    // Total Expenses from expense_details (summing all relevant item fields)
    query.exec("SELECT SUM(COALESCE(item_subtotal, 0) + COALESCE(item_tax, 0) + COALESCE(item_shipping, 0) - COALESCE(item_promotion, 0)) AS total_expenses FROM expense_details");
    if (!query.isActive()) {
        qDebug() << "Total expenses query error:" << query.lastError().text();
    }
    double totalExpenses = 0.0;
    if (query.next()) {
        QVariant value = query.value("total_expenses");
        if (!value.isNull()) totalExpenses = value.toDouble();

    }
    ui->totalExpensesLabel->setText(QString("Total Expenses: $%1").arg(totalExpenses, 0, 'f', 2));
}

void ReportsPage::updateCustomerSpending()
{
    QString queryStr = "SELECT CONCAT(c.first_name, ' ', c.last_name) AS customer_name, "
                       "SUM(o.grand_total) AS total_spent "
                       "FROM orders o JOIN customer c ON o.customer_ID = c.customer_ID "
                       "WHERE o.status = 'Paid' "
                       "GROUP BY o.customer_ID "
                       "ORDER BY total_spent DESC";
    customerSpendingModel->setQuery(queryStr, db);
    if (customerSpendingModel->lastError().isValid()) {
        qDebug() << "Customer spending query error:" << customerSpendingModel->lastError().text();
    }

    customerSpendingModel->setHeaderData(0, Qt::Horizontal, "Customer Name");
    customerSpendingModel->setHeaderData(1, Qt::Horizontal, "Total Spent");
    ui->customerSpendingTable->resizeColumnsToContents();
    ui->customerSpendingTable->viewport()->update();
}

void ReportsPage::updateCandleSales()
{
    // Updated query to use Order_Items, Product_Details, Product, and Orders
    QString queryStr = "SELECT p.product_name, SUM(oi.qty) AS quantity_sold "
                       "FROM order_items oi "
                       "JOIN product_details pd ON oi.detail_ID = pd.detail_ID "
                       "JOIN product p ON pd.product_ID = p.product_ID "
                       "JOIN orders o ON oi.order_ID = o.order_ID "
                       "WHERE o.status = 'Paid' "
                       "GROUP BY pd.product_ID "
                       "ORDER BY quantity_sold DESC";
    candleSalesModel->setQuery(queryStr, db);
    if (candleSalesModel->lastError().isValid()) {
        qDebug() << "Candle sales query error:" << candleSalesModel->lastError().text();
    }

    candleSalesModel->setHeaderData(0, Qt::Horizontal, "Product Name");
    candleSalesModel->setHeaderData(1, Qt::Horizontal, "Quantity Sold");
    ui->candleSalesTable->resizeColumnsToContents();
    ui->candleSalesTable->viewport()->update();

    // Total Candles Sold (updated query)
    QSqlQuery query(db);
    query.exec("SELECT SUM(oi.qty) AS total_sold "
               "FROM order_items oi "
               "JOIN orders o ON oi.order_ID = o.order_ID "
               "WHERE o.status = 'Paid'");
    if (!query.isActive()) {
        qDebug() << "Total candles query error:" << query.lastError().text();
    }
    int totalSold = 0;
    if (query.next()) {
        QVariant value = query.value("total_sold");
        if (!value.isNull()) totalSold = value.toInt();

    }
    ui->totalCandlesLabel->setText(QString("Total Candles Sold: %1").arg(totalSold));
}
