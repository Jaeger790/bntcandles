#ifndef CUSTOMERMODEL_H
#define CUSTOMERMODEL_H

#include <QSqlTableModel>
#include <QSqlDatabase>

class CustomerModel : public QSqlTableModel{
    Q_OBJECT

public:
    explicit CustomerModel(const QSqlDatabase &db, QObject *parent = nullptr);
    //set up headers, filter, and other custom table options
    void configure();

};


#endif // CUSTOMERMODEL_H
