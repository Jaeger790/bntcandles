#include "reportspage.h"
#include "ui_reportspage.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>  // For timestamps if needed

ReportsPage::ReportsPage(const QSqlDatabase &db, QWidget *parent)
    : QWidget(parent), ui(new Ui::ReportsPage), db(db)
{
    qDebug() << "ReportsPage: DB open:" << db.isOpen();
    ui->setupUi(this);

    // Grab browser (from .ui)
    reportsBrowser = ui->reportsBrowser;
    if (!reportsBrowser) {
        qCritical() << "CRITICAL: reportsBrowser null—check .ui!";
        return;
    }
    reportsBrowser->setOpenExternalLinks(false);  // No ext nav needed

    // Legacy models (stubbed; remove later)
    customerSpendingModel = new QSqlQueryModel(this);
    candleSalesModel = new QSqlQueryModel(this);

    refreshReports();  // Initial load
}

ReportsPage::~ReportsPage()
{
    delete ui;
    delete customerSpendingModel;
    delete candleSalesModel;
}

void ReportsPage::refreshReports()
{
    updateTotals();
    updateCustomerSpending();
    updateCandleSales();
    buildFullReportHtml();  // New: Render all
}

void ReportsPage::updateTotals()
{
    QSqlQuery query(db);

    // Total Income (unchanged query)
    query.exec("SELECT SUM(grand_total) AS total_income FROM orders WHERE status = 'Paid'");
    if (!query.isActive()) {
        qDebug() << "Total income error:" << query.lastError().text();
    }
    double totalIncome = 0.0;
    if (query.next() && !query.value("total_income").isNull()) {
        totalIncome = query.value("total_income").toDouble();
    }

    // Total Expenses (unchanged)
    query.exec("SELECT SUM(COALESCE(item_subtotal, 0) + COALESCE(item_tax, 0) + COALESCE(item_shipping, 0) - COALESCE(item_promotion, 0)) AS total_expenses FROM expense_details");
    if (!query.isActive()) {
        qDebug() << "Total expenses error:" << query.lastError().text();
    }
    double totalExpenses = 0.0;
    if (query.next() && !query.value("total_expenses").isNull()) {
        totalExpenses = query.value("total_expenses").toDouble();
    }

    // New: Store for HTML (class members? Or pass via signals; here globals for sim)
    // TODO: Add private QString totalIncomeHtml, totalExpensesHtml; for cleaner
    qDebug() << "Totals: Income=" << totalIncome << ", Expenses=" << totalExpenses;

    // Temp: Build mini-HTML here (move to buildFull if complex)
    // But for now, we'll glue in buildFullReportHtml()
}

void ReportsPage::updateCustomerSpending()
{
    QString queryStr = "SELECT CONCAT(c.first_name, ' ', c.last_name) AS customer_name, "
                       "SUM(o.grand_total) AS total_spent "
                       "FROM orders o JOIN customer c ON o.customer_ID = c.customer_ID "
                       "WHERE o.status = 'Paid' "
                       "GROUP BY o.customer_ID "
                       "ORDER BY total_spent DESC";
    customerSpendingModel->setQuery(queryStr, db);  // Legacy
    if (customerSpendingModel->lastError().isValid()) {
        qDebug() << "Customer spending error:" << customerSpendingModel->lastError().text();
    }

    // New: Build table HTML
    QString tableHtml = "<table class='report-table'><thead><tr><th>Customer Name</th><th>Total Spent ($)</th></tr></thead><tbody>";
    int rowCount = 0;
    QSqlQuery spendQuery(db);  // Fresh query for HTML loop
    spendQuery.exec(queryStr);
    while (spendQuery.next()) {
        QString name = spendQuery.value("customer_name").toString().toHtmlEscaped();
        double spent = spendQuery.value("total_spent").toDouble();
        tableHtml += QString("<tr><td>%1</td><td>%2</td></tr>")
                        .arg(name).arg(spent, 0, 'f', 2);
        rowCount++;
    }
    tableHtml += "</tbody></table><p class='row-count'>Rows: " + QString::number(rowCount) + "</p>";

    qDebug() << "Customer spending: Loaded" << rowCount << "rows";
    // Store: Add private QString customerTableHtml; = tableHtml;
}

void ReportsPage::updateCandleSales()
{
    QString queryStr = "SELECT p.product_name, SUM(oi.qty) AS quantity_sold "
                       "FROM order_items oi "
                       "JOIN product_details pd ON oi.detail_ID = pd.detail_ID "
                       "JOIN product p ON pd.product_ID = p.product_ID "
                       "JOIN orders o ON oi.order_ID = o.order_ID "
                       "WHERE o.status = 'Paid' "
                       "GROUP BY pd.product_ID "
                       "ORDER BY quantity_sold DESC";

    candleSalesModel->setQuery(queryStr, db);  // Legacy
    if (candleSalesModel->lastError().isValid()) {
        qDebug() << "Candle sales error:" << candleSalesModel->lastError().text();
    }

    // New: Total Candles (unchanged query, but HTML-ify)
    QSqlQuery totalQuery(db);
    totalQuery.exec("SELECT SUM(oi.qty) AS total_sold "
                    "FROM order_items oi "
                    "JOIN orders o ON oi.order_ID = o.order_ID "
                    "WHERE o.status = 'Paid'");
    int totalSold = 0;
    if (totalQuery.next() && !totalQuery.value("total_sold").isNull()) {
        totalSold = totalQuery.value("total_sold").toInt();
    }

    // Build table HTML
    QString tableHtml = "<table class='report-table'><thead><tr><th>Product Name</th><th>Quantity Sold</th></tr></thead><tbody>";
    int rowCount = 0;
    QSqlQuery salesQuery(db);
    salesQuery.exec(queryStr);
    while (salesQuery.next()) {
        QString name = salesQuery.value("product_name").toString().toHtmlEscaped();
        int qty = salesQuery.value("quantity_sold").toInt();
        tableHtml += QString("<tr><td>%1</td><td>%2</td></tr>")
                        .arg(name).arg(qty);
        rowCount++;
    }
    tableHtml += "</tbody></table><p class='row-count'>Total Candles Sold: " + QString::number(totalSold) + " | Rows: " + QString::number(rowCount) + "</p>";

    qDebug() << "Candle sales: Loaded" << rowCount << "rows, total sold:" << totalSold;
    // Store: Add private QString candleTableHtml; = tableHtml;
}

// New: Glue everything into full HTML doc
void ReportsPage::buildFullReportHtml() {
    // Fetch totals (re-query for freshness; optimize later)
    QSqlQuery totalsQuery(db);
    double totalIncome = 0.0;
    totalsQuery.exec("SELECT SUM(grand_total) AS total_income FROM orders WHERE status = 'Paid'");
    if (totalsQuery.next()) totalIncome = totalsQuery.value("total_income").toDouble();

    double totalExpenses = 0.0;
    totalsQuery.exec("SELECT SUM(COALESCE(item_subtotal, 0) + COALESCE(item_tax, 0) + COALESCE(item_shipping, 0) - COALESCE(item_promotion, 0)) AS total_expenses FROM expense_details");
    if (totalsQuery.next()) totalExpenses = totalsQuery.value("total_expenses").toDouble();

    // Re-build tables (call updates first? Or inline here for sim; refactor to privates)
    // For brevity: Inline customer table (copy from updateCustomerSpending)
    QString customerTable = "<table class='report-table'><thead><tr><th>Customer Name</th><th>Total Spent ($)</th></tr></thead><tbody>";
    QSqlQuery customerQuery(db);
    customerQuery.exec("SELECT CONCAT(c.first_name, ' ', c.last_name) AS customer_name, "
                       "SUM(o.grand_total) AS total_spent "
                       "FROM orders o JOIN customer c ON o.customer_ID = c.customer_ID "
                       "WHERE o.status = 'Paid' "
                       "GROUP BY o.customer_ID "
                       "ORDER BY total_spent DESC");
    while (customerQuery.next()) {
        customerTable += QString("<tr><td>%1</td><td>%2</td></tr>")
                            .arg(customerQuery.value("customer_name").toString().toHtmlEscaped())
                            .arg(customerQuery.value("total_spent").toDouble(), 0, 'f', 2);
    }
    customerTable += "</tbody></table>";

    // Inline candle table (similar)
    QString candleTable = "<table class='report-table'><thead><tr><th>Product Name</th><th>Quantity Sold</th></tr></thead><tbody>";
    QSqlQuery candleQuery(db);
    candleQuery.exec("SELECT p.product_name, SUM(oi.qty) AS quantity_sold "
                     "FROM order_items oi JOIN product_details pd ON oi.detail_ID = pd.detail_ID "
                     "JOIN product p ON pd.product_ID = p.product_ID "
                     "JOIN orders o ON oi.order_ID = o.order_ID "
                     "WHERE o.status = 'Paid' "
                     "GROUP BY pd.product_ID "
                     "ORDER BY quantity_sold DESC");
    while (candleQuery.next()) {
        candleTable += QString("<tr><td>%1</td><td>%2</td></tr>")
                           .arg(candleQuery.value("product_name").toString().toHtmlEscaped())
                           .arg(candleQuery.value("quantity_sold").toInt());
    }
    candleTable += "</tbody></table>";

    // Full doc template
    QString fullHtml = R"(
        <html>
        <head>
            <style>
                body {
                    font-family: 'Segoe UI', Roboto, Arial, sans-serif;
                    margin: 0;
                    padding: 30px;
                    color: #ecf0f1;
                    background: rgba(34, 0, 43, 0.66);;
                }

                h1 {
                    text-align: center;
                    color: #00bcd4;
                    margin-bottom: 40px;
                    font-size: 28px;

                }

                /* Top summary cards */

                
                .total-card {
                    

                    padding:300px;
                    text-align: center;
     
 
                }
                .card{
                    border-bottom: 1px solid rgba(255,255,255,0.8);
                    border-right: 1px solid rgba(255,255,255,0.8);
                }

                .total-label {
                      color: #00bcd4;
                    border-bottom: 1px solid rgba(255,255,255,0.2);
                    padding-bottom: 8px;
                    margin-bottom: 15px;
                    font-size: 20px;
                }

                .total-value {
                    font-size: 26px;
                    font-weight: bold;
                    color: #2ecc71;
                }


                table.dahsboard {
                    width: 100%;
                    border-collapse: collapse;
                    border-radius: 8px;
                    overflow: hidden;
                }
                
                table.table-grid {
                    width: 45%;
                    border-collapse: collapse;
                    overflow: hidden;
                    vertical-align: top;
                    margin: 10px;
                }
    
                 
                td, th {
                    width: 100%;
                    padding: 20px;
                    text-align: left;
                    

                }
                td{
                    font-size: 16px;
                    border-bottom: 1px solid rgba(255,255,255,0.2);
                   
                }
                .card {
                    
                    padding: 20px;
                    border: 1px solid rgba(255,255,255,0.2);
                    
                }
                th {
                    background: #1b2838;
                    color: #00bcd4;
                    text-transform: uppercase;
                    font-size: 20px;
                    letter-spacing: 0.5px;
                }
             

            </style>
        </head>
        <body>
            <h1>BNT Candles Dashboard</h1>
           
            <table class='dashboard'>
                <tr>
                    <td>
                        <div class="total-card">
                            <div class="total-label">Total Candles Sold</div>
                            <div class="total-value">%1</div>
                        </div>
                    </td>
                    
                    <td>
                        <div class="total-card">
                            <div class="total-label">Total Income</div>
                            <div class="total-value">$%2</div>
                        </div>
                    </td>

                    <td>
                        <div class="total-card">
                            <div class="total-label">Total Expenses</div>
                            <div class="total-value">$%3</div>
                        </div>
                    </td>
                </tr>
            </table>
            

            
            <table class='table-grid'>
                <tr>
                    <td>
                        <div class="card">
                            <h2>Customer Spending</h2>
                            %4
                        </div>
                    </td>
                    <td>
                        <div class="card">
                            <h2>Candle Sales by Product</h2>
                            %5
                        </div>
                    </td>
                </tr>
            </table>
            
        </body>
        </html>
    )";

    // Calc total sold (from earlier query)
    QSqlQuery soldQuery(db);
    int totalSold = 0;
    soldQuery.exec("SELECT SUM(oi.qty) AS total_sold FROM order_items oi JOIN orders o ON oi.order_ID = o.order_ID WHERE o.status = 'Paid'");
    if (soldQuery.next()) totalSold = soldQuery.value("total_sold").toInt();

    // Inject (order: totalSold, income, expenses, customerTable, candleTable, timestamp)
    fullHtml = fullHtml.arg(totalSold)
                       .arg(totalIncome, 0, 'f', 2)
                       .arg(totalExpenses, 0, 'f', 2)
                       .arg(customerTable)
                       .arg(candleTable)
                       .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    // Render & Debug
    reportsBrowser->setHtml(fullHtml);
    qDebug() << "Full report HTML len:" << fullHtml.length() << "| Rendered:" << reportsBrowser->toHtml().length();
    if (fullHtml.contains("Customer Spending")) {
        qDebug() << "✅ Report rendered—check browser.";
    } else {
        qWarning() << "❌ HTML build fail. Snippet:" << fullHtml.left(200);
        reportsBrowser->setHtml("<div class='error'>Report generation error—check DB queries.</div>");
    }

    reportsBrowser->viewport()->update();  // Force paint
}