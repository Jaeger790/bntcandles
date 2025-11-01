#ifndef ADDEXPENSEDETAILWINDOW_H
#define ADDEXPENSEDETAILWINDOW_H

#include <QDialog>

namespace Ui {
class AddExpenseDetailWindow;
}

class AddExpenseDetailWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AddExpenseDetailWindow(QWidget *parent = nullptr);
    ~AddExpenseDetailWindow();

    QString date() const;
    QString description() const;
    QString category() const;
    QString merchant() const;
    QString itemName() const;
    int quantity() const;
    double unitPrice() const;
    double itemSubtotal() const;
    double itemTax() const;
    double itemShipping() const;
    double itemPromotion() const;
    double itemTaxRate() const;  // Returns % (e.g., 8.5)
    QString notes() const;

    void setDate(const QString &date);
    void setDescription(const QString &description);
    void setCategory(const QString &category);
    void setMerchant(const QString &merchant);
    void setItemName(const QString &itemName);
    void setQuantity(int quantity);
    void setUnitPrice(double price);
    void setItemTaxRate(double taxRate);  // taxRate as %
    void setNotes(const QString &notes);

private:
    Ui::AddExpenseDetailWindow *ui;
};

#endif // ADDEXPENSEDETAILWINDOW_H