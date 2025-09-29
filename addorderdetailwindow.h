#ifndef ADDORDERDETAILWINDOW_H
#define ADDORDERDETAILWINDOW_H

#include <QDialog>
#include <QSqlDatabase>
#include <QtSql/QSqlTableModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class AddOrderDetailWindow;
}
QT_END_NAMESPACE

class AddOrderDetailWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AddOrderDetailWindow(const QSqlDatabase &db, QWidget *parent = nullptr);
    ~AddOrderDetailWindow();

    // Getters
    int detailId() const;
    int quantity() const;
    double unitPrice() const;
    double taxRate() const;
    double subtotal() const;
    double taxAmount() const;
    double total() const;

    // Setters for editing
    void setDetailId(int detailId);
    void setQuantity(int quantity);
    void setUnitPrice(double unitPrice);
    void setTaxRate(double taxRate);
    void setSubtotal(double subtotal);
    void setTaxAmount(double taxAmount);
    void setTotal(double total);

private:
    Ui::AddOrderDetailWindow *ui;
    QSqlDatabase db;
};

#endif // ADDORDERDETAILWINDOW_H
