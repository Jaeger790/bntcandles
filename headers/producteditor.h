#ifndef PRODUCTEDITOR_H
#define PRODUCTEDITOR_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlTableModel>

namespace Ui { class ProductEditor; }

class ProductEditor : public QDialog {
    Q_OBJECT

public:
    explicit ProductEditor(const QString &productId, 
    const QSqlDatabase &db, QWidget *parent = nullptr); 
    ~ProductEditor();

private slots:
    void saveProductName();
    void addDetail();
    void editDetail();
    void deleteDetail();
    void saveChanges();  

private:
    void updateTotalStock();  
    Ui::ProductEditor *ui;
    QSqlTableModel *detailsModel;
    QString productId;  
    QSqlDatabase db;
    bool isNewProduct;
};

#endif // PRODUCTEDITOR_H