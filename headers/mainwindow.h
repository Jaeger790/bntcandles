#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

// Forward declarations (Fixed: Global scope, before class)
class ReportsPage;
class CustomerPage;
class ProductPage;
class OrderPage;
class ExpensePage;

QT_BEGIN_NAMESPACE
namespace Ui { class BNTcandles; }
QT_END_NAMESPACE

class BNTcandles : public QMainWindow
{
    Q_OBJECT

public:
    BNTcandles(QWidget *parent = nullptr);
    ~BNTcandles();

private:
    Ui::BNTcandles *ui;
    QSqlDatabase db;

    // Page instances (now resolves via global fwds)
    ReportsPage *reportsPage;
    CustomerPage *customerPage;
    ProductPage *productPage;
    OrderPage *orderPage;
    ExpensePage *expensePage;
};

#endif // MAINWINDOW_H