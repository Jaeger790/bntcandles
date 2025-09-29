#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QDate>
#include <QSqlQuery>

QT_BEGIN_NAMESPACE
namespace Ui { class BNTcandles; }
QT_END_NAMESPACE

class ReportsPage;

class BNTcandles : public QMainWindow
{
    Q_OBJECT

public:
    BNTcandles(QWidget *parent = nullptr);
    ~BNTcandles();

private slots:
    void customerButtonClicked();
    void productButtonClicked();
    void orderButtonClicked();
    void expenseButtonClicked();
    void reportButtonClicked();

    void addCustomer();
    void editCustomer();
    void deleteCustomer();

    void addProduct();
    void editProduct();
    void deleteProduct();

    void addOrder();
    void editOrder();
    void deleteOrder();

    void addExpense();
    void editExpense();
    void deleteExpense();

private:
    Ui::BNTcandles *ui;
    QSqlTableModel *customerModel;
    QSqlTableModel *productModel;
    QSqlTableModel *orderModel;
    QSqlTableModel *expenseModel;
    ReportsPage *reportsPage;
    QSqlDatabase db;
};

#endif // MAINWINDOW_H
