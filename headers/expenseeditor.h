#ifndef EXPENSEEDITOR_H
#define EXPENSEEDITOR_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlTableModel>

namespace Ui {
class ExpenseEditor;
}

class AddExpenseDetailWindow;

class ExpenseEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ExpenseEditor(const QString &orderId, const QSqlDatabase &db, QWidget *parent = nullptr);
    ~ExpenseEditor();

private slots:
    void saveOrderInfo();
    void addItem();
    void editItem();
    void deleteItem();
    void saveAndClose();

private:
    Ui::ExpenseEditor *ui;
    QSqlTableModel *itemsModel;
    QString orderId;
    QSqlDatabase db;
    bool isNewOrder;

    void populateCategoryCombo();
    void populatePaymentMethodCombo();  // New: For order-level payment
    void loadOrderInfo();
    void saveChanges();
    void addDetail();
    void editDetail();
    void deleteDetail();
};

#endif // EXPENSEEDITOR_H