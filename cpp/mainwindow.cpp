/*
 * This class opens the connection to the database, sets up the pages for the tabs,
 * connects ui elements from main window, and establishes logic for page navigation.
 */
#include "../headers/mainwindow.h"
#include "../ui/ui_mainwindow.h"

// Full page includes (Fixed: Provides complete types for new/insert/delete)
#include "../headers/reportspage.h"
#include "../headers/customerpage.h"
#include "../headers/productpage.h"
#include "../headers/orderpage.h"
#include "../headers/expensepage.h"

#include <QSqlError>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QIcon>  // For setWindowIcon

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

    // Instantiate pages (after DB open) - Now with full types
    reportsPage = new ReportsPage(db, this);
    customerPage = new CustomerPage(db, this);
    productPage = new ProductPage(db, this);
    orderPage = new OrderPage(db, this);
    expensePage = new ExpensePage(db, this);

    // Add to stack: reports (0), customer (1), product (2), order (3), expense (4)
    ui->menuStack->insertWidget(0, reportsPage);
    ui->menuStack->insertWidget(1, customerPage);
    ui->menuStack->insertWidget(2, productPage);
    ui->menuStack->insertWidget(3, orderPage);
    ui->menuStack->insertWidget(4, expensePage);

    // Connect navigation buttons to appropriate pages
    connect(ui->reportButton, &QPushButton::clicked, [this]() { ui->menuStack->setCurrentIndex(0); });
    connect(ui->customerButton, &QPushButton::clicked, [this]() { ui->menuStack->setCurrentIndex(1); });
    connect(ui->productButton, &QPushButton::clicked, [this]() { ui->menuStack->setCurrentIndex(2); });
    connect(ui->orderButton, &QPushButton::clicked, [this]() { ui->menuStack->setCurrentIndex(3); });
    connect(ui->expenseButton, &QPushButton::clicked, [this]() { ui->menuStack->setCurrentIndex(4); });

    // Set initial page to reports/home
    ui->menuStack->setCurrentIndex(0);
}

BNTcandles::~BNTcandles()
{
    delete reportsPage;
    delete customerPage;
    delete productPage;
    delete orderPage;
    delete expensePage;
    delete ui;
}