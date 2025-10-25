#ifndef ADDCUSTOMERWINDOW_H
#define ADDCUSTOMERWINDOW_H

#include <QDialog>

namespace Ui {
class AddCustomerWindow;
}

class AddCustomerWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AddCustomerWindow(QWidget *parent = nullptr);
    ~AddCustomerWindow();

    //Getters
    QString firstName() const;
    QString lastName() const;
    QString phone() const;
    QString email() const;
    QString address() const;
    QString city() const;
    QString state() const;
    QString zip() const;
    QString company() const;

    //Setters
    void setFirstName(const QString &firstName);
    void setLastName(const QString &lastName);
    void setPhone(const QString &phone);
    void setEmail(const QString &email);
    void setAddress(const QString &address);
    void setCity(const QString &city);
    void setState(const QString &state);
    void setZip(const QString &zip);
    void setCompany(const QString &company);



private:
    Ui::AddCustomerWindow *ui;
};

#endif // ADDCUSTOMERWINDOW_H
