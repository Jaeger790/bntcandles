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

private slots:
    void loadHTML();

private:
    Ui::AddOrderDetailWindow *ui;
    QSqlDatabase db;

    // New: Member vars for computed values (since UI fields removed)
    int m_detailId = 0;
    double m_unitPrice = 0.0;
    double m_taxRate = 0.0;
    double m_subtotal = 0.0;
    double m_taxAmount = 0.0;
    double m_total = 0.0;
};

#endif // ADDORDERDETAILWINDOW_H