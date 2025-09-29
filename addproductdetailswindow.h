#ifndef ADDPRODUCTDETAILSWINDOW_H
#define ADDPRODUCTDETAILSWINDOW_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class AddProductDetailsWindow; }
QT_END_NAMESPACE

class AddProductDetailsWindow : public QDialog {
    Q_OBJECT

public:
    explicit AddProductDetailsWindow(QWidget *parent = nullptr);
    ~AddProductDetailsWindow();

    // Getters
    QString size() const;
    double price() const;
    double taxRate() const;
    int stockQuantity() const;

    // Setters
    void setSize(const QString &size);
    void setPrice(double price);
    void setTaxRate(double taxRate);
    void setStockQuantity(int stockQty);

private:
    Ui::AddProductDetailsWindow *ui;
};

#endif // ADDPRODUCTDETAILSWINDOW_H