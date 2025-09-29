#ifndef ADDORDERWINDOW_H
#define ADDORDERWINDOW_H

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class AddOrderWindow;
}

class AddOrderWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AddOrderWindow(const QSqlDatabase &db, QWidget *parent = nullptr);
    ~AddOrderWindow();

    // Getters
    int customerId() const;
    QDate orderDate() const;
    double totalAmount() const;
    QString status() const;

    // Setters
    void setCustomerId(int customerId);
    void setOrderDate(const QDate &date);
    void setTotalAmount(double amount);
    void setStatus(const QString &status);

private:
    Ui::AddOrderWindow *ui;
    QSqlDatabase db;
    void populateCustomers();  // Method to load customers into combo box
};

#endif // ADDORDERWINDOW_H