#ifndef PRODUCTEDITOR_H
#define PRODUCTEDITOR_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlTableModel>

namespace Ui { class ProductEditor; }

class ProductEditor : public QDialog {
    Q_OBJECT

public:
    explicit ProductEditor(const QString &productId, const QSqlDatabase &db, QWidget *parent = nullptr);  // Empty productId for add
    ~ProductEditor();

private slots:
    void saveProductName();
    void addDetail();
    void editDetail();
    void deleteDetail();
    void saveChanges();  // Connected to Save button

private:
    void updateTotalStock();  // If not using DB trigger
    Ui::ProductEditor *ui;
    QSqlTableModel *detailsModel;
    QString productId;  // Empty for new product
    QSqlDatabase db;
    bool isNewProduct;
};

#endif // PRODUCTEDITOR_H