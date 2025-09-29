#ifndef EXPENSEDETAILSWINDOW_H
#define EXPENSEDETAILSWINDOW_H

#include <QDialog>
#include <QSqlDatabase>
#include <QtSql/QSqlTableModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class ExpenseDetailsWindow;
}
QT_END_NAMESPACE

class ExpenseDetailsWindow : public QDialog {  // Remove Ui::
    Q_OBJECT

public:
    explicit ExpenseDetailsWindow(const QString &orderId, const QSqlDatabase &db, QWidget *parent = nullptr);
    ~ExpenseDetailsWindow();

private slots:
    void addDetail();
    void editDetail();
    void deleteDetail();

private:
    Ui::ExpenseDetailsWindow *ui;
    QSqlTableModel *expenseDetailsModel;
    QString orderId;
    QSqlDatabase db;
};

#endif // EXPENSEDETAILSWINDOW_H
