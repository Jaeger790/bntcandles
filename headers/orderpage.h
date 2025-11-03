#ifndef ORDERPAGE_H
#define ORDERPAGE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>  // Fixed: SQl → QSql

namespace Ui {
class OrderPage;
}

class OrderPage : public QWidget
{
    Q_OBJECT

public:
    explicit OrderPage(const QSqlDatabase &db, QWidget *parent = nullptr);
    ~OrderPage();

private slots:
    void addOrder();
    void editOrder();
    void deleteOrder();

private:
    Ui::OrderPage *ui;  // Added: Missing member
    QSqlDatabase db;    // Added: Missing member
    QSqlTableModel *orderModel;  // Added: Missing member

    void setupOrderTable();  // Added: Missing declaration
    void refreshTable();     // Added: Missing declaration
};

#endif // ORDERPAGE_H