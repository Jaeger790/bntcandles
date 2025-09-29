#ifndef REPORTSPAGE_H
#define REPORTSPAGE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>

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

    Ui::ReportsPage *ui;
    QSqlDatabase db;
    QSqlQueryModel *customerSpendingModel;
    QSqlQueryModel *candleSalesModel;
};

#endif // REPORTSPAGE_H