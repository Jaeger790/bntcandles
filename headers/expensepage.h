#ifndef EXPENSEPAGE_H
#define EXPENSEPAGE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>

namespace Ui {
class ExpensePage;
}

class ExpensePage : public QWidget
{
    Q_OBJECT

public:
    explicit ExpensePage(const QSqlDatabase &db, QWidget *parent = nullptr);
    ~ExpensePage();

private slots:
    void addExpense();
    void editExpense();
    void deleteExpense();

private:
    Ui::ExpensePage *ui;
    QSqlDatabase db;
    QSqlTableModel *expenseModel;

    void setupExpenseTable();
    void refreshTable();
};

#endif // EXPENSEPAGE_H