#ifndef REPORTSPAGE_H
#define REPORTSPAGE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>  // Legacy—remove post-test
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QChartView>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QLabel>
#include <QGridLayout>
#include <QGraphicsDropShadowEffect>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

namespace Ui {
class ReportsPage;
}

class ReportsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ReportsPage(const QSqlDatabase &db, QWidget *parent = nullptr);
    ~ReportsPage();

public slots:
    void refreshReports();

private:
    void updateTotals();
    void updateCustomerSpending();  // Now builds chart data only
    void updateCandleSales();       // Now builds chart data only
    void updateDashboard();         // Updates labels from DB
    void setupDashboard();          // Applies shadows (embedded in UI)
    void setupCandleSalesChart();
    void setupCustomerSpendingChart();

    Ui::ReportsPage *ui;
    QSqlDatabase db;
    QSqlQueryModel *customerSpendingModel;  // Legacy
    QSqlQueryModel *candleSalesModel;       // Legacy
    QChartView *candleSalesChartView;
    QChartView *customerSpendingChartView;
    QWidget *candlePlaceholder;
    QWidget *customerPlaceholder;

    // Dashboard members (access via ui-> in setup/update)
    QWidget *dashboardWidget;
    QLabel *totalSoldLabel;
    QLabel *totalIncomeLabel;
    QLabel *totalExpensesLabel;
    QLabel *netProfitLabel;
    QLabel *dashboardTitle;
};

#endif // REPORTSPAGE_H