#ifndef PRODUCTPAGE_H
#define PRODUCTPAGE_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlTableModel>

namespace Ui{
    class ProductPage;
}

class ProductPage : public QWidget {
    Q_OBJECT

public:
    explicit ProductPage(const QSqlDatabase &db , QWidget *parent = nullptr);
    ~ProductPage();

private slots:
    void addProduct();
    void editProduct();
    void deleteProduct();


private:
    Ui::ProductPage *ui;
    QSqlDatabase db;
    QSqlTableModel *productModel;

    void refreshTable();
    void setupProductTable();
};




#endif //PRODUCTPAGE_H