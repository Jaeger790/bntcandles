// expenseeditor.h
#ifndef EXPENSEEDITOR_H
#define EXPENSEEDITOR_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QDate>
#include <QSqlQuery>

QT_BEGIN_NAMESPACE
namespace Ui { class ExpenseEditor; }
QT_END_NAMESPACE

class AddExpenseDetailWindow;

class ExpenseEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ExpenseEditor(const QString &orderId, const QSqlDatabase &db, QWidget *parent = nullptr);
    ~ExpenseEditor();

private slots:
    void populateCategoryCombo();
    void saveOrderInfo();
    void saveChanges();
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

    void loadOrderInfo();
    void addDetail();
    void editDetail();
    void deleteDetail();
};

#endif // EXPENSEEDITOR_H