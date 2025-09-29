#ifndef PRODUCTDETAILSWINDOW_H
#define PRODUCTDETAILSWINDOW_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlTableModel>

QT_BEGIN_NAMESPACE
namespace Ui { class ProductDetailsWindow; }
QT_END_NAMESPACE

class ProductDetailsWindow : public QDialog {
    Q_OBJECT

public:
    explicit ProductDetailsWindow(const QString &productId, const QSqlDatabase &db, QWidget *parent = nullptr);
    ~ProductDetailsWindow();

private slots:
    void addDetail();
    void editDetail();
    void deleteDetail();

private:
    Ui::ProductDetailsWindow *ui;
    QSqlTableModel *productDetailsModel;
    QString productId;
    QSqlDatabase db;
};

#endif // PRODUCTDETAILSWINDOW_H
