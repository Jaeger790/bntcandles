#ifndef ADDEXPENSE_H
#define ADDEXPENSE_H

#include <QDialog>
#include <QSqlDatabase>
#include <QDate>

namespace Ui {
class AddExpense;
}

class AddExpense : public QDialog
{
    Q_OBJECT

public:
    explicit AddExpense(QSqlDatabase &db, QWidget *parent = nullptr);
    ~AddExpense();

    void setEditMode(bool edit) { m_editMode = edit; }
    void setOrderIdDisplay(const QString &orderId); // Ensure this is present
    void setDate(const QDate &date);
    void setPaymentMethod(const QString &paymentMethod);
    void setSource(const QString &source);
    void setDescription(const QString &description);
    void setCategory(const QString &category);
    void setItemName(const QString &itemName);
    void setQuantity(int quantity);
    void setItemSubtotal(double subtotal);
    void setItemTax(double tax);
    void setItemShipping(double shipping);
    void setItemPromotion(double promotion);
    void setItemTaxRate(double taxRate);
    void setNotes(const QString &notes);

    QDate date() const;
    QString paymentMethod() const;
    QString source() const;
    QString description() const;
    QString category() const;
    QString itemName() const;
    int quantity() const;
    double itemSubtotal() const;
    double itemTax() const;
    double itemShipping() const;
    double itemPromotion() const;
    double itemTaxRate() const;
    QString notes() const;

private slots:
    void accept() override;

private:
    Ui::AddExpense *ui;
    QSqlDatabase &m_db;
    bool m_editMode = false;
};

#endif // ADDEXPENSE_H
