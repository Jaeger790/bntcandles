#ifndef ORDERPAGE_H
#define ORDERPAGE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>

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
    Ui::OrderPage *ui;
    QSqlDatabase db;
    QSqlTableModel *orderModel;

    void setupOrderTable();
    void refreshTable();
};

#endif // ORDERPAGE_H
