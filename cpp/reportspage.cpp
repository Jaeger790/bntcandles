#include "../headers/reportspage.h"
#include "../ui/ui_reportspage.h"
#include <QSqlQuery>
#include <QString>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QVector>
#include <QPainter>  

ReportsPage::ReportsPage(const QSqlDatabase &db, QWidget *parent)
    : QWidget(parent), ui(new Ui::ReportsPage), db(db)
{
    ui->setupUi(this);
    ui->scrollArea->setFrameShape(QFrame::NoFrame);
    candlePlaceholder = ui->candleChartPlaceholder;
    customerPlaceholder = ui->customerChartPlaceholder;

    // Legacy models 
    customerSpendingModel = new QSqlQueryModel(this);
    candleSalesModel = new QSqlQueryModel(this);

    // Init charts 
    candleSalesChartView = nullptr;
    customerSpendingChartView = nullptr;

    // Setup layouts for placeholders
    auto candleLayout = new QVBoxLayout(candlePlaceholder);
    candleLayout->setContentsMargins(0, 0, 0, 0);
    auto customerLayout = new QVBoxLayout(customerPlaceholder);
    customerLayout->setContentsMargins(0, 0, 0, 0);
    
    setupDashboard();
    refreshReports();  
}

ReportsPage::~ReportsPage()
{
    delete ui;
    delete customerSpendingModel;
    delete candleSalesModel;
}

void ReportsPage::setupDashboard()
{
    // Embedded in UI—map original names via ui->
    dashboardWidget = ui->dashboardWidget;
    dashboardTitle = ui->dashboardTitle;
    totalSoldLabel = ui->totalSoldLabel;
    totalIncomeLabel = ui->totalIncomeLabel;
    totalExpensesLabel = ui->totalExpensesLabel;
    netProfitLabel = ui->netProfitLabel;

    // Apply drop shadows to cards
    auto applyShadow = [this](QWidget *card) {
        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
        effect->setBlurRadius(10);
        effect->setColor(QColor(255, 255, 255, 78));
        effect->setOffset(0, 2);
        card->setGraphicsEffect(effect);
    };
    applyShadow(ui->totalSoldCard);
    applyShadow(ui->totalIncomeCard);
    applyShadow(ui->totalExpensesCard);
    applyShadow(ui->netProfitCard);
}

void ReportsPage::refreshReports()
{
    updateTotals();
    updateCustomerSpending();  // Prepares data for chart
    updateCandleSales();       // Prepares data for chart
    updateDashboard();         // Updates QLabel values
    setupCandleSalesChart();   // Build and add chart
    setupCustomerSpendingChart();
}

void ReportsPage::updateTotals()
{
    QSqlQuery query(db);
    // Total Income (unchanged)
    query.exec("SELECT SUM(grand_total) AS total_income FROM orders WHERE status = 'Complete'");
    if (!query.isActive()) {
    }
    double totalIncome = 0.0;
    if (query.next() && !query.value("total_income").isNull()) {
        totalIncome = query.value("total_income").toDouble();
    }
    // Total Expenses (unchanged)
    query.exec("SELECT SUM(COALESCE(item_subtotal, 0) + COALESCE(item_tax, 0) + COALESCE(item_shipping, 0) - COALESCE(item_promotion, 0)) AS total_expenses FROM expense_details");
    if (!query.isActive()) {
    }
    double totalExpenses = 0.0;
    if (query.next() && !query.value("total_expenses").isNull()) {
        totalExpenses = query.value("total_expenses").toDouble();
    }
    double netProfit = totalIncome - totalExpenses;
}

void ReportsPage::updateCustomerSpending()
{
    QString queryStr = "SELECT CONCAT(c.first_name, ' ', c.last_name) AS customer_name, "
                       "SUM(o.grand_total) AS total_spent "
                       "FROM orders o JOIN customer c ON o.customer_ID = c.customer_ID "
                       "WHERE o.status = 'Complete' "
                       "GROUP BY o.customer_ID "
                       "ORDER BY total_spent DESC LIMIT 10";  // Limit for chart
    customerSpendingModel->setQuery(queryStr, db);
    if (customerSpendingModel->lastError().isValid()) {
    }
}

void ReportsPage::updateCandleSales()
{
    QString queryStr = "SELECT p.product_name, SUM(oi.qty) AS quantity_sold "
                       "FROM order_items oi "
                       "JOIN product_details pd ON oi.detail_ID = pd.detail_ID "
                       "JOIN product p ON pd.product_ID = p.product_ID "
                       "JOIN orders o ON oi.order_ID = o.order_ID "
                       "WHERE o.status = 'Complete' "
                       "GROUP BY pd.product_ID "
                       "ORDER BY quantity_sold DESC LIMIT 10";  // Limit for chart
    candleSalesModel->setQuery(queryStr, db);
    if (candleSalesModel->lastError().isValid()) {
    }
}

void ReportsPage::updateDashboard()
{
    // Fetch totals
    QSqlQuery totalsQuery(db);
    double totalIncome = 0.0;
    totalsQuery.exec("SELECT SUM(grand_total) AS total_income FROM orders WHERE status = 'Complete'");
    if (totalsQuery.next()) totalIncome = totalsQuery.value("total_income").toDouble();
    double totalExpenses = 0.0;
    totalsQuery.exec("SELECT SUM(COALESCE(item_subtotal, 0) + COALESCE(item_tax, 0) + COALESCE(item_shipping, 0) - COALESCE(item_promotion, 0)) AS total_expenses FROM expense_details");
    if (totalsQuery.next()) totalExpenses = totalsQuery.value("total_expenses").toDouble();
    double netProfit = totalIncome - totalExpenses;
    // Total sold
    QSqlQuery soldQuery(db);
    int totalSold = 0;
    soldQuery.exec("SELECT SUM(oi.qty) AS total_sold FROM order_items oi JOIN orders o ON oi.order_ID = o.order_ID WHERE o.status = 'Complete'"); 
    if (soldQuery.next()) totalSold = soldQuery.value("total_sold").toInt();
    // Update labels (access via ui->)
    ui->totalSoldLabel->setText(QString::number(totalSold));
    ui->totalIncomeLabel->setText(QString("$%1").arg(totalIncome, 0, 'f', 2));
    ui->totalExpensesLabel->setText(QString("$%1").arg(totalExpenses, 0, 'f', 2));
    ui->netProfitLabel->setText(QString("$%1").arg(netProfit, 0, 'f', 2));
    // Dynamic styling for profit (negative = red)
    if (netProfit < 0) {
        ui->netProfitLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #e74c3c; border: 0;");
    } else {
        ui->netProfitLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #fafafa; border: 0;");
    }
}

void ReportsPage::setupCandleSalesChart()
{
    // Clear existing
    if (candleSalesChartView) {
        delete candleSalesChartView;
    }
    auto layout = qobject_cast<QVBoxLayout*>(candlePlaceholder->layout());
    if (layout && candleSalesChartView) {
        layout->removeWidget(candleSalesChartView);
    }
    // Fetch all data upfront 
    while (candleSalesModel->canFetchMore()) {
        candleSalesModel->fetchMore();
    }
    QVector<QString> categories;
    QVector<int> quantities;
    for (int i = 0; i < candleSalesModel->rowCount() && i < 10; ++i) {
        QModelIndex nameIdx = candleSalesModel->index(i, 0);
        QModelIndex qtyIdx = candleSalesModel->index(i, 1);
        QString name = candleSalesModel->data(nameIdx).toString();
        int qty = candleSalesModel->data(qtyIdx).toInt();
        categories << name;
        quantities << qty;
    }
    if (!categories.isEmpty()) {
        QBarSeries *series = new QBarSeries();
        QBarSet *set = new QBarSet("Quantity Sold");
        // Bulk append (efficient for small N=10)
        for (int qty : quantities) {
            *set << qty;
        }
        series->append(set);
        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Candle Sales by Product");
        chart->setAnimationOptions(QChart::SeriesAnimations);
        QFont titleFont;
        titleFont.setBold(true);
        titleFont.setPixelSize(24);
        chart->setTitleFont(titleFont);
        chart->setTitleBrush(QBrush(QColor(255,255,255,255)));
        chart->legend()->setLabelColor(QColor(255,255,255,255));
        // Set background colors
        chart->setBackgroundVisible(false);
        chart->setPlotAreaBackgroundVisible(true);
        chart->setPlotAreaBackgroundBrush(QBrush(QColor(218, 33, 106,25)));

        set->setColor(QColor(52,15,100,255));
        

        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        // Bulk append categories
        for (const QString& cat : categories) {
            axisX->append(cat);
        }
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Quantity Sold");
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        QFont labelsFont;
        QBrush labelsBrush(Qt::white);
        labelsFont.setPixelSize(14);
        axisX->setLabelsBrush(labelsBrush);
        axisX->setLabelsFont(labelsFont);
        axisY->setLabelsBrush(labelsBrush);
        axisY->setLabelsFont(labelsFont);

        axisY->setGridLineVisible(true);
        axisY->setShadesPen(Qt::NoPen);
        axisY->setShadesBrush(QBrush(QColor(218,66,156,50)));
        axisY->setShadesVisible(false);

        candleSalesChartView = new QChartView(chart, this);
        candleSalesChartView->setRenderHint(QPainter::Antialiasing);
        candleSalesChartView->setMinimumSize(400,600);
        axisX->setLabelsAngle(-45);
        layout->addWidget(candleSalesChartView);
    } else {
        auto label = new QLabel("No candle sales data available.", this);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }
}

void ReportsPage::setupCustomerSpendingChart()
{
    // Clear existing
    if (customerSpendingChartView) {
        delete customerSpendingChartView;
    }
    auto layout = qobject_cast<QVBoxLayout*>(customerPlaceholder->layout());
    if (layout && customerSpendingChartView) {
        layout->removeWidget(customerSpendingChartView);
    }
    // Fetch all data upfront )
    while (customerSpendingModel->canFetchMore()) {
        customerSpendingModel->fetchMore();
    }
    QVector<QString> categories;
    QVector<double> amounts;
    for (int i = 0; i < customerSpendingModel->rowCount() && i < 10; ++i) {
        QModelIndex nameIdx = customerSpendingModel->index(i, 0);
        QModelIndex spentIdx = customerSpendingModel->index(i, 1);
        QString name = customerSpendingModel->data(nameIdx).toString();
        double spent = customerSpendingModel->data(spentIdx).toDouble();
        categories << name;
        amounts << spent;
    }
    if (!categories.isEmpty()) {
        QBarSeries *series = new QBarSeries();
        QBarSet *set = new QBarSet("Total Spent ($)");
        for (double amt : amounts) {
            *set << amt;
        }
        series->append(set);
        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Customer Spending");
        chart->setAnimationOptions(QChart::SeriesAnimations);
        QFont titleFont;
        titleFont.setBold(true);
        titleFont.setPixelSize(24);
        chart->setTitleFont(titleFont);
        chart->setTitleBrush(QBrush(QColor(255,255,255,255)));
        chart->legend()->setLabelColor(QColor(255,255,255,255));
        // Set background colors
        chart->setBackgroundVisible(false);
        chart->setPlotAreaBackgroundVisible(true);
        chart->setPlotAreaBackgroundBrush(QBrush(QColor(218, 33, 106,25)));
        set->setColor(QColor(52,15,100,255));
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        // Bulk append categories
        for (const QString& cat : categories) {
            axisX->append(cat);
        }
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);
        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Total Spent ($)");
        axisY->setTitleBrush(QBrush(QColor(255,255,255,255)));
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);
        QFont labelsFont;
        QBrush labelsBrush(Qt::white);
        labelsFont.setPixelSize(14);
        axisX->setLabelsBrush(labelsBrush);
        axisX->setLabelsFont(labelsFont);
        axisY->setLabelsBrush(labelsBrush);
        axisY->setLabelsFont(labelsFont);
        axisX->setGridLineVisible(true);
        axisY->setGridLineVisible(true);
        axisY->setGridLineColor(QColor(255,255,255,255));
        customerSpendingChartView = new QChartView(chart, this);
        customerSpendingChartView->setRenderHint(QPainter::Antialiasing);
        customerSpendingChartView->setMinimumSize(400,600);
        axisX->setLabelsAngle(-45);
        layout->addWidget(customerSpendingChartView);
   
    } else {
        auto label = new QLabel("No customer spending data available.", this);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }
}