#ifndef ADDEXPENSEDETAILWINDOW_H
#define ADDEXPENSEDETAILWINDOW_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class AddExpenseDetailWindow;
}
QT_END_NAMESPACE


class AddExpenseDetailWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AddExpenseDetailWindow(QWidget *parent = nullptr);
    ~AddExpenseDetailWindow();

    // Getters
    QString date() const;
    QString description() const;
    QString category() const;
    QString paymentMethod() const;
    QString source() const;
    QString itemName() const;
    int quantity() const;
    double itemSubtotal() const;
    double itemTax() const;
    double itemShipping() const;
    double itemPromotion() const;
    double itemTaxRate() const;
    QString notes() const;

    // Setters for editing
    void setDate(const QString &date);
    void setDescription(const QString &description);
    void setCategory(const QString &category);
    void setPaymentMethod(const QString &paymentMethod);
    void setSource(const QString &source);
    void setItemName(const QString &itemName);
    void setQuantity(int quantity);
    void setItemSubtotal(double subtotal);
    void setItemTax(double tax);
    void setItemShipping(double shipping);
    void setItemPromotion(double promotion);
    void setItemTaxRate(double taxRate);
    void setNotes(const QString &notes);

private:
    Ui::AddExpenseDetailWindow *ui;
};

#endif // ADDEXPENSEDETAILWINDOW_H
