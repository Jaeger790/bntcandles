#ifndef ORDERDETAILSWINDOW_H
#define ORDERDETAILSWINDOW_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlRelationalTableModel> // Changed to QSqlRelationalTableModel

QT_BEGIN_NAMESPACE
namespace Ui {
class OrderDetailsWindow;
}
QT_END_NAMESPACE

class OrderDetailsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit OrderDetailsWindow(const QString &orderId, const QSqlDatabase &db, QWidget *parent = nullptr);
    ~OrderDetailsWindow();

private slots:
    void addDetail();
    void editDetail();
    void deleteDetail();

private:
    Ui::OrderDetailsWindow *ui;
    QSqlRelationalTableModel *orderItemsModel; // Changed to QSqlRelationalTableModel
    QString orderId;
    QSqlDatabase db;
};

#endif // ORDERDETAILSWINDOW_H
