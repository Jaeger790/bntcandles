/*
 * This calss opens the connection to the database, sets up the models for the tables being fetched from database,
 * connects ui elements from main window, and establishes logic for table manipulation and page navigation.
    TODO: Seperate pages and functions into seperate classes.  Will be handled like reports/home pagge. 10/09/25   
 */
#include "../headers/mainwindow.h"        
#include "../ui/ui_mainwindow.h"
#include "../headers/addcustomer.h"
#include "../headers/reportspage.h"
#include "../headers/producteditor.h"
#include "../headers/ordereditor.h"
#include "../headers/expenseeditor.h"
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QGraphicsDropShadowEffect>

BNTcandles::BNTcandles(QWidget *parent) : QMainWindow(parent), ui(new Ui::BNTcandles){
    ui->setupUi(this);
    setWindowTitle("BNT Candles");
    setWindowIcon(QIcon(":/bnt_icon.jpg"));

    // Enable styled background
    setAttribute(Qt::WA_StyledBackground, true);
    // Load stylesheet from resources
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)){
        QTextStream stream(&styleFile);
        QString stylesheet = stream.readAll();
        setStyleSheet(stylesheet);
        styleFile.close();
    }else{
        QMessageBox::warning(this,"Failed to load stylesheet", "Failed to load stylesheet for MainWindow.");
    }

/*      // Production DB credentials
    QSettings settings(":/db_cred.ini", QSettings::IniFormat);
    QString host = settings.value("prod_db/host").toString();
    int port = settings.value("prod_db/port").toInt();
    QString dbName = settings.value("prod_db/name").toString();
    QString user = settings.value("prod_db/user").toString();
    QString password = settings.value("prod_db/password").toString(); */

    /*  //Laptop Test DB credentials
    QSettings settings(":/db_cred.ini", QSettings::IniFormat);
    QString host = settings.value("brit_test_db/host").toString();
    int port = settings.value("brit_test_db/port").toInt();
    QString dbName = settings.value("brit_test_db/name").toString();
    QString user = settings.value("brit_test_db/user").toString();
    QString password = settings.value("brit_test_db/password").toString(); */
     

    //Test DB credentials
    QSettings settings(":/db_cred.ini", QSettings::IniFormat);
    QString host = settings.value("test_db/host").toString();
    int port = settings.value("test_db/port").toInt();
    QString dbName = settings.value("test_db/name").toString();
    QString user = settings.value("test_db/user").toString();
    QString password = settings.value("test_db/password").toString(); 

    // Set up database connection
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(host);
    db.setPort(port);
    db.setDatabaseName(dbName);
    db.setUserName(user);
    db.setPassword(password);

    // Error message if connection fails
    if (!db.open()){
        QMessageBox::critical(this, "Database error", "Failed to connect: " + db.lastError().text());
        return;
    }

    // Create customer table model
    customerModel = new QSqlTableModel(this, db);
    customerModel->setTable("customer");
    customerModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    customerModel->setHeaderData(1, Qt::Horizontal, "First Name");
    customerModel->setHeaderData(2, Qt::Horizontal, "Last Name");
    customerModel->setHeaderData(3, Qt::Horizontal, "Phone #");
    customerModel->setHeaderData(4, Qt::Horizontal, "Email");
    customerModel->setHeaderData(5, Qt::Horizontal, "Address");
    customerModel->setHeaderData(6, Qt::Horizontal, "City");
    customerModel->setHeaderData(7, Qt::Horizontal, "State");
    customerModel->setHeaderData(8, Qt::Horizontal, "Zip Code");
    customerModel->setHeaderData(9, Qt::Horizontal, "Company");

    if (!customerModel->select()){
        QMessageBox::warning(this, "Data Error", "Failed to load customers: " + customerModel->lastError().text());
    }

    // Connect customer table to ui
    ui->customerTable->setModel(customerModel);
    ui->customerTable->setColumnHidden(0, true); // Hide customer_id
    ui->customerTable->resizeColumnsToContents();
    ui->customerTable->horizontalHeader()->setStretchLastSection(false);
    ui->customerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->customerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->customerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    customerModel->select(); // Force data fetch
    ui->customerTable->viewport()->update();

    // Connect Customer Buttons to ui
    connect(ui->addCustomerButton, &QPushButton::clicked, this, &BNTcandles::addCustomer);
    connect(ui->editCustomerButton, &QPushButton::clicked, this, &BNTcandles::editCustomer);
    connect(ui->deleteCustomerButton, &QPushButton::clicked, this, &BNTcandles::deleteCustomer);

    // Create product model
    productModel = new QSqlTableModel(this, db);
    productModel->setTable("product");
    productModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    productModel->setHeaderData(1, Qt::Horizontal, "Product Name");
    productModel->setHeaderData(2, Qt::Horizontal, "Description");
    productModel->setHeaderData(2, Qt::Horizontal, "Total Stock");

    if (!productModel->select()){
        QMessageBox::warning(this, "Data Error", "Failed to load PRODCUTS: " + customerModel->lastError().text());
    }

    // Connect Product Table to ui
    ui->productTable->setModel(productModel);
    ui->productTable->setColumnHidden(0, true); // Hide product_id
    ui->productTable->resizeColumnsToContents();
    ui->productTable->horizontalHeader()->setStretchLastSection(false);
    ui->productTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->productTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->productTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Connect Product Buttons to ui
    connect(ui->addProductButton, &QPushButton::clicked, this, &BNTcandles::addProduct);
    connect(ui->editProductButton, &QPushButton::clicked, this, &BNTcandles::editProduct);
    connect(ui->deleteProductButton, &QPushButton::clicked, this, &BNTcandles::deleteProduct);
    
    // Connect the double click function to table ui
    connect(ui->productTable, &QTableView::doubleClicked, this, [this](const QModelIndex &index){
        ui->productTable->selectRow(index.row()); //table selection
    
        // Get the product_ID from the selected row (column 0)
        QString productId = productModel->data(productModel->index(index.row(), 0)).toString();

        // Open ProductDetailsWindow
        ProductEditor productEditor(productId, productModel->database(), this);
        productEditor.exec(); 
    });

    // Create order model with join on customer to display custom view
    orderModel = new QSqlTableModel(this, db);
    orderModel->setTable("ordersview");
    orderModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    orderModel->setHeaderData(0, Qt::Horizontal, "Order ID");
    orderModel->setHeaderData(1, Qt::Horizontal, "First Name"); // from customer table linked by custId.
    orderModel->setHeaderData(2, Qt::Horizontal, "Last Name"); // from customer table
    orderModel->setHeaderData(3, Qt::Horizontal, "Order Date");
    orderModel->setHeaderData(4, Qt::Horizontal, "Status");
    orderModel->setHeaderData(5, Qt::Horizontal, "Subtotal");
    orderModel->setHeaderData(6, Qt::Horizontal, "Tax Amount");
    orderModel->setHeaderData(7, Qt::Horizontal, "Grand Total");
    orderModel->setHeaderData(8, Qt::Horizontal, "Payment Method");
    // bool selectSuccess = orderModel->select();

    if (!orderModel->select()){
        QMessageBox::warning(this, "Data Error", "Failed to load expenses: " + orderModel->lastError().text());
    }

    // Connect Order Table to ui
    ui->orderTable->setModel(orderModel);
    ui->orderTable->setColumnHidden(0, false); // Hide order_id or not
    ui->orderTable->resizeColumnsToContents();
    ui->orderTable->horizontalHeader()->setStretchLastSection(false);
    ui->orderTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->orderTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->orderTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Connect orderTable double-click to open OrderDetailsWindow
    connect(ui->orderTable, &QTableView::doubleClicked, this, [this](const QModelIndex &index){
        ui->orderTable->selectRow(index.row()); // Table selection
        QString orderId = orderModel->data(orderModel->index(index.row(), 0)).toString();
        OrderEditor editor(orderId, this->db, this);  
        if (editor.exec() == QDialog::Accepted) {
            orderModel->select(); 
            
        }
    });

    // Connect buttons
    connect(ui->addOrderButton, &QPushButton::clicked, this, &BNTcandles::addOrder);
    connect(ui->editOrderButton, &QPushButton::clicked, this, &BNTcandles::editOrder);
    connect(ui->deleteOrderButton, &QPushButton::clicked, this, &BNTcandles::deleteOrder);

    // Create expense model
    expenseModel = new QSqlTableModel(this, db);
    expenseModel->setTable("expense_orders");
    expenseModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    expenseModel->setHeaderData(0, Qt::Horizontal, "Order ID");
    expenseModel->setHeaderData(1, Qt::Horizontal, "Date");
    expenseModel->setHeaderData(2, Qt::Horizontal, "Total");
    expenseModel->setHeaderData(3, Qt::Horizontal, "Payment Method");
    expenseModel->setHeaderData(4, Qt::Horizontal, "Company");

    if (!expenseModel->select()){
        QMessageBox::warning(this, "Data Error", "Failed to load expenses: " + expenseModel->lastError().text());
    }

    // Connect Expense Table to ui
    ui->expenseTable->setModel(expenseModel);
    ui->expenseTable->setColumnHidden(10, true); // Hide Created At column
    ui->expenseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->expenseTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->expenseTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->expenseTable->resizeColumnsToContents();
    ui->expenseTable->horizontalHeader()->setStretchLastSection(false);

    // Connect expenseTable double-click to open ExpenseDetailsWindow
    connect(ui->expenseTable, &QTableView::doubleClicked, this, [this](const QModelIndex &index){
        QString orderId = expenseModel->data(expenseModel->index(index.row(), 0)).toString();
        ExpenseEditor *dialog = new ExpenseEditor(orderId, this->db, this);
        dialog->exec();
        delete dialog; // Clean up to prevent memory leak
    });

    // Connect Expense buttons
    connect(ui->addExpenseButton, &QPushButton::clicked, this, &BNTcandles::addExpense);
    connect(ui->editExpenseButton, &QPushButton::clicked, this, &BNTcandles::editExpense);
    connect(ui->deleteExpenseButton, &QPushButton::clicked, this, &BNTcandles::deleteExpense);

    // Set up reports page
    reportsPage = new ReportsPage(db, this);
    ui->menuStack->insertWidget(0, reportsPage);

    // Connect navigation buttons to appropriate pages
    connect(ui->customerButton, &QPushButton::clicked, this, &BNTcandles::customerButtonClicked);
    connect(ui->productButton, &QPushButton::clicked, this, &BNTcandles::productButtonClicked);
    connect(ui->orderButton, &QPushButton::clicked, this, &BNTcandles::orderButtonClicked);
    connect(ui->expenseButton, &QPushButton::clicked, this, &BNTcandles::expenseButtonClicked);
    connect(ui->reportButton, &QPushButton::clicked, this, &BNTcandles::reportButtonClicked);

    /*
    QList<QPushButton*> buttons = this->findChildren<QPushButton*>();
    for (QPushButton *btn : buttons) {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(btn);
        shadow->setBlurRadius(30);
        shadow->setXOffset(5);
        shadow->setYOffset(5);
        shadow->setColor(QColor(177,156,217,250));
        btn->setGraphicsEffect(shadow);
    }
     */
    // Set initial page to customer page
    ui->menuStack->setCurrentIndex(0);
}
/*
 * Customer Functions: Add, Edit, and delete
 * Add Customer
 */
void BNTcandles::addCustomer()
{
    AddCustomerWindow dialog(this);
    if (dialog.exec() == QDialog::Accepted){
        QSqlRecord record = customerModel->record();
        record.setValue("first_name", dialog.firstName());
        record.setValue("last_name", dialog.lastName());
        record.setValue("phone", dialog.phone());
        record.setValue("email", dialog.email());
        record.setValue("address_street", dialog.address());
        record.setValue("address_city", dialog.city());
        record.setValue("address_state", dialog.state());
        record.setValue("address_zip", dialog.zip());
        record.setValue("company_name", dialog.company());
        if (customerModel->insertRecord(-1, record)){
            if (customerModel->submitAll()){
                ui->customerTable->resizeColumnsToContents();
            }else{
                QMessageBox::warning(this, "Error", "Failed to add customer record: " + customerModel->lastError().text());
            }
        }else{
            QMessageBox::warning(this, "Error", "Failed to add customer record: " + customerModel->lastError().text());
        }
    }
}

/*
 * Edit Customer
*/
void BNTcandles::editCustomer(){
    QModelIndexList selection = ui->customerTable->selectionModel()->selectedRows();
    if (selection.isEmpty()){
        QMessageBox::warning(this, "Selection Error", "Please select a customer from the table to edit.");
        return;
    }
    int row = selection.first().row();
    QSqlRecord record = customerModel->record(row);
    AddCustomerWindow dialog(this);
    dialog.setWindowTitle("Edit Customer");
    dialog.setFirstName(record.value("first_name").toString());
    dialog.setLastName(record.value("last_name").toString());
    dialog.setPhone(record.value("phone").toString());
    dialog.setEmail(record.value("email").toString());
    dialog.setAddress(record.value("address_street").toString());
    dialog.setCity(record.value("address_city").toString());
    dialog.setState(record.value("address_state").toString());
    dialog.setZip(record.value("address_zip").toString());
    dialog.setCompany(record.value("company_name").toString());
    if (dialog.exec() == QDialog::Accepted){
        record.setValue("first_name", dialog.firstName());
        record.setValue("last_name", dialog.lastName());
        record.setValue("phone", dialog.phone());
        record.setValue("email", dialog.email());
        record.setValue("address_street", dialog.address());
        record.setValue("address_city", dialog.city());
        record.setValue("address_state", dialog.state());
        record.setValue("address_zip", dialog.zip());
        record.setValue("company_name", dialog.company());
        if (customerModel->setRecord(row, record)){
            if (customerModel->submitAll()){
                ui->customerTable->resizeColumnsToContents();
            }else{
                QMessageBox::warning(this, "Error", "Failed to update customer: " + customerModel->lastError().text());
            }
        }else{
            QMessageBox::warning(this, "Error", "Failed to set customer record: " + customerModel->lastError().text());
        }
    }
}

/*
 * Delete Customer
*/
void BNTcandles::deleteCustomer(){
    QModelIndexList selection = ui->customerTable->selectionModel()->selectedRows();
    if (selection.isEmpty()){
        QMessageBox::warning(this, "Selection Error", "Please select a csutomer from the table to delete");
    }
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you wish to delete the selected customer", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes){
        std::sort(selection.begin(), selection.end(), [](const QModelIndex &a, const QModelIndex &b){
                      return a.row() > b.row();
                    });

        for (const auto &index : selection){
            customerModel->removeRow(index.row());
        }
        if (customerModel->submitAll()){
            ui->customerTable->resizeColumnsToContents();
        }else{
            QMessageBox::warning(this, "Error", "Failed to delete customer: " + customerModel->lastError().text());
        }
    }
}

/*
 * Product Fucntions: Add,Edit, and Delete
 * Add Product
*/
void BNTcandles::addProduct()
{
    ProductEditor dialog("", db, this); // Empty ID for add
    if (dialog.exec() == QDialog::Accepted)
    {
        productModel->select();
        ui->productTable->resizeColumnsToContents();
    }
}

void BNTcandles::editProduct()
{
    QModelIndexList selection = ui->productTable->selectionModel()->selectedRows();
    if (selection.isEmpty())
    {
        QMessageBox::warning(this, "Selection Error", "Please select a product to edit.");
        return;
    }
    int row = selection.first().row();
    QString productId = productModel->data(productModel->index(row, 0)).toString();
    ProductEditor dialog(productId, db, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        productModel->select();
        ui->productTable->resizeColumnsToContents();
    }
}

void BNTcandles::deleteProduct()
{
    QModelIndexList selection = ui->productTable->selectionModel()->selectedRows();
    if (selection.isEmpty())
    {
        QMessageBox::warning(this, "Selection Error", "Please select a product to delete.");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete the selected product(s) and all associated details?",
        QMessageBox::Yes | QMessageBox::No
    );
    if (reply == QMessageBox::Yes)
    {
        QSqlQuery query;
        std::sort(selection.begin(), selection.end(), [](const QModelIndex &a, const QModelIndex &b)
                  {
                      return a.row() > b.row();
                  });

        for (const auto &index : selection)
        {
            QString productId = productModel->data(productModel->index(index.row(), 0)).toString();
            // Check for Order_Items dependencies
            query.prepare("SELECT COUNT(*) FROM product_details pd "
                          "JOIN order_items oi ON pd.detail_ID = oi.detail_ID "
                          "WHERE pd.product_ID = :productId");
            query.bindValue(":productId", productId);
            if (!query.exec())
            {
                QMessageBox::warning(this, "Error", "Failed to check dependencies: " + query.lastError().text());
                return;
            }
            query.next();
            if (query.value(0).toInt() > 0)
            {
                QMessageBox::warning(this, "Error", "Cannot delete product: Associated details are referenced in Order_Items.");
                return;
            }
            // Delete Product_Details records
            query.prepare("DELETE FROM product_details WHERE product_ID = :productId");
            query.bindValue(":productId", productId);
            if (!query.exec())
            {
                QMessageBox::warning(this, "Error", "Failed to delete product details: " + query.lastError().text());
                return;
            }
            // Delete Product record
            if (!productModel->removeRow(index.row()))
            {
                QMessageBox::warning(this, "Error", "Failed to remove product record.");
                return;
            }
        }

        if (productModel->submitAll())
        {
            ui->productTable->resizeColumnsToContents();
        }
        else
        {
            QMessageBox::warning(this, "Error", "Failed to delete product: " + productModel->lastError().text());
        }
    }
}

void BNTcandles::addOrder()
{
    OrderEditor dialog("", db, this); // Empty ID for add
    if (dialog.exec() == QDialog::Accepted)
    {
        orderModel->select();
        ui->orderTable->resizeColumnsToContents();
    }
}

void BNTcandles::editOrder(){
    QModelIndexList selection = ui->orderTable->selectionModel()->selectedRows();
    if (selection.isEmpty()){
        QMessageBox::warning(this, "Selection Error", "Please select an order to edit.");
        return;
    }
    int row = selection.first().row();
    QString orderId = orderModel->data(orderModel->index(row, 0)).toString();
    OrderEditor dialog(orderId, db, this);
    
    if (dialog.exec() == QDialog::Accepted){
        orderModel->select();
        ui->orderTable->resizeColumnsToContents();
    }
}

void BNTcandles::deleteOrder()

{

    QModelIndexList selection = ui->orderTable->selectionModel()->selectedRows();
    if (selection.isEmpty())
    {
        QMessageBox::warning(this, "Selection Error", "Please select an order to delete.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete the selected order(s)? This will also delete associated order details.",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes)
    {
        QSqlQuery query(db);
        bool success = true;
        std::sort(selection.begin(), selection.end(), [](const QModelIndex &a, const QModelIndex &b)
                  {
                      return a.row() > b.row();
                  });

        for (const auto &index : selection)
        {
            int orderId = orderModel->data(orderModel->index(index.row(), 0)).toInt();
            // First, delete associated order details
            query.prepare("DELETE FROM order_items WHERE order_ID = :orderId");
            query.bindValue(":orderId", orderId);
            if (!query.exec())
            {
                success = false;
                QMessageBox::warning(this, "Database Error", "Failed to delete order details: " + query.lastError().text());
                break;
            }
            // Then, delete the order
            query.prepare("DELETE FROM `orders` WHERE order_ID = :orderId");
            query.bindValue(":orderId", orderId);
            if (!query.exec())
            {
                success = false;
                QMessageBox::warning(this, "Database Error", "Failed to delete order: " + query.lastError().text());
                break;
            }
        }
        if (success)
        {
            orderModel->select(); // Refresh the table model
            ui->orderTable->resizeColumnsToContents();
        }
    }
}

void BNTcandles::addExpense()
{
    ExpenseEditor editor("", db, this);  // Empty ID = new
    if (editor.exec() == QDialog::Accepted) {
        expenseModel->select();  // Refresh expense table
    } else {
        qDebug() << "Add cancelled";
    }
}

void BNTcandles::editExpense()
{
    QModelIndexList selection = ui->expenseTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select an expense to edit.");
        return;
    }
    int row = selection.first().row();
    QString orderId = expenseModel->data(expenseModel->index(row, 0)).toString();  // Col 0 = order_id
    ExpenseEditor editor(orderId, db, this);
    if (editor.exec() == QDialog::Accepted) {
        expenseModel->select();  // Refresh
    } else {
        qDebug() << "Edit cancelled";
    }
}

void BNTcandles::deleteExpense(){
    QModelIndexList selection = ui->expenseTable->selectionModel()->selectedRows();
    if (selection.isEmpty()){
        QMessageBox::warning(this, "Selection Error", "Please select an expense order to delete.");
        return;
    }
    int row = selection.first().row();
    QString orderId = expenseModel->data(expenseModel->index(row, 0)).toString();
    // Confirm deletion
    int reply = QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this order and all its details?", QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;
    QSqlQuery query(db);
    query.prepare("DELETE FROM expense_orders WHERE order_id = :orderId");
    query.bindValue(":orderId", orderId);
    if (query.exec()){
        expenseModel->select(); // Refresh table
    }else{
        QMessageBox::warning(this, "Database Error", "Failed to delete order: " + query.lastError().text());
    }
}

void BNTcandles::reportButtonClicked(){
    ui->menuStack->setCurrentIndex(0);
}
void BNTcandles::customerButtonClicked(){
    ui->menuStack->setCurrentIndex(2);
}

void BNTcandles::productButtonClicked(){
    ui->menuStack->setCurrentIndex(3);
}

void BNTcandles::orderButtonClicked(){
    ui->menuStack->setCurrentIndex(4);
}

void BNTcandles::expenseButtonClicked(){
    ui->menuStack->setCurrentIndex(5);
}

BNTcandles::~BNTcandles()

{

    delete customerModel;

    delete productModel;

    delete orderModel;

    delete expenseModel;

    delete reportsPage;

    delete ui;
}
