#ifndef EXPENSEEDITOR_H
#define EXPENSEEDITOR_H

#include <QDialog>
#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QSqlRecord>
#include "../headers/addexpensedetailwindow.h"  // For param

QT_BEGIN_NAMESPACE
namespace Ui { class ExpenseEditor; }
QT_END_NAMESPACE

class ExpenseEditor : public QDialog
{
    Q_OBJECT

public:
    ExpenseEditor(const QString &orderId, const QSqlDatabase &db, QWidget *parent = nullptr);
    ~ExpenseEditor();

private slots:
    void updateRunningTotal();

private:
    struct OrderTotals {
        double subtotal = 0.0;
        double tax = 0.0;
        double shipping = 0.0;
        double promotion = 0.0;
        double grandTotal() const { return subtotal + tax + shipping - promotion; }
    };

    void populateCategoryCombo();
    void populatePaymentMethodCombo();
    void loadOrderInfo();
    void saveOrderInfo();
    void saveChanges();
    void saveAndClose();
    void addDetail();
    void editDetail();
    void deleteDetail();
    void updateUIState();
    void refreshTable();
    void populateRecordFromDialog(QSqlRecord &record, const AddExpenseDetailWindow &dialog, bool isEdit = false);
    OrderTotals calculateOrderTotals() const;

    void addItem() { addDetail(); }
    void editItem() { editDetail(); }
    void deleteItem() { deleteDetail(); }

    Ui::ExpenseEditor *ui;
    QSqlTableModel *itemsModel;
    QString orderId;
    QSqlDatabase db;
    bool isNewOrder;
};

#endif // EXPENSEEDITOR_H