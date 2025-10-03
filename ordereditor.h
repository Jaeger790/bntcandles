#ifndef ORDEREDITOR_H
#define ORDEREDITOR_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlTableModel>

namespace Ui { class OrderEditor; }

class OrderEditor : public QDialog {
    Q_OBJECT

public:
    explicit OrderEditor(const QString &orderId,
    const QSqlDatabase &db, QWidget *parent = nullptr);
    ~OrderEditor();

private slots:
    
    void addItem();
    void editItem();
    void deleteItem();
    void saveAndClose();  


private:
    void populateCustomerCombo();
    void populateStatusCombo();
    void saveOrderDetails();
    
    Ui::OrderEditor *ui;
    QSqlTableModel *itemsModel;
    QString orderId;
    QSqlDatabase db;
    bool isNewOrder;
};

#endif // ORDEREDITOR_H