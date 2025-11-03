#ifndef CUSTOMERPAGE_H
#define CUSTOMERPAGE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>

namespace Ui {
class CustomerPage;
}

class CustomerPage : public QWidget
{
    Q_OBJECT

public:
    explicit CustomerPage(const QSqlDatabase &db, QWidget *parent = nullptr);  // Fixed: const for consistency
    ~CustomerPage();

private slots:
    void addCustomer();
    void editCustomer();
    void deleteCustomer();

private:
    Ui::CustomerPage *ui;
    QSqlDatabase db;
    QSqlTableModel *customerModel;

    void setupCustomerTable();
    void refreshTable();  // Added: Missing decl
};

#endif // CUSTOMERPAGE_H