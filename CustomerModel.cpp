#include "CustomerModel.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlError>


CustomerModel::CustomerModel(const QSqlDatabase &db, QObject *parent) : QSqlTableModel(parent,db){
    setTable("Customer");
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    configure();
}

void CustomerModel::configure(){
    setHeaderData(1,Qt::Horizontal,"First Name");
    setHeaderData(2,Qt::Horizontal,"Last Name");
    setHeaderData(3,Qt::Horizontal,"Phone Number");
    setHeaderData(4,Qt::Horizontal, "Email");
    setHeaderData(5,Qt::Horizontal,"Address");
    setHeaderData(6,Qt::Horizontal,"City");
    setHeaderData(7,Qt::Horizontal,"State");
    setHeaderData(8,Qt::Horizontal,"Zip Code");
    setHeaderData(9,Qt::Horizontal,"Company");
    if(!select()){
        qDebug() << "Customer model error:" << lastError().text();
        QMessageBox::warning(nullptr, "Data Error", "Failed to load customer data: " + lastError().text());
    }
}
