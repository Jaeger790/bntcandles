#ifndef REPORTSPAGE_H
#define REPORTSPAGE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>  // Keep for now (deprecate later)
#include <QTextBrowser>    // New: For HTML rendering

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
    void updateCustomerSpending();
    void updateCandleSales();
    void buildFullReportHtml();  // New: Assembles all sections into one doc

    Ui::ReportsPage *ui;
    QSqlDatabase db;
    QSqlQueryModel *customerSpendingModel;  // Legacy—remove post-test
    QSqlQueryModel *candleSalesModel;       // ^
    QTextBrowser *reportsBrowser;  // New: Single pane for everything
};

#endif // REPORTSPAGE_H