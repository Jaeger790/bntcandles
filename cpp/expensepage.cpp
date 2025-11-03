#include "../headers/expensepage.h"
#include "../ui/ui_expensepage.h"
#include "../headers/expenseeditor.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>  // Added: For lastError().text()
#include <algorithm>  // For std::sort (for multi-delete)

ExpensePage::ExpensePage(const QSqlDatabase &db, QWidget *parent)
    : QWidget(parent), ui(new Ui::ExpensePage), db(db)
{
    ui->setupUi(this);

    expenseModel = new QSqlTableModel(this, db);
    expenseModel->setTable("expense_orders");
    expenseModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    expenseModel->setHeaderData(0, Qt::Horizontal, "Order ID");
    expenseModel->setHeaderData(1, Qt::Horizontal, "Date");
    expenseModel->setHeaderData(2, Qt::Horizontal, "Total");
    expenseModel->setHeaderData(3, Qt::Horizontal, "Payment Method");
    expenseModel->setHeaderData(4, Qt::Horizontal, "Company");

    setupExpenseTable();
    refreshTable();

    // Connect buttons
    connect(ui->addExpenseButton, &QPushButton::clicked, this, &ExpensePage::addExpense);
    connect(ui->editExpenseButton, &QPushButton::clicked, this, &ExpensePage::editExpense);
    connect(ui->deleteExpenseButton, &QPushButton::clicked, this, &ExpensePage::deleteExpense);

    // Double-click to edit (Fixed: capture &db)
    connect(ui->expenseTable, &QTableView::doubleClicked, this, [this, &db](const QModelIndex &index) {
        ui->expenseTable->selectRow(index.row());
        QString orderId = expenseModel->data(expenseModel->index(index.row(), 0)).toString();
        ExpenseEditor *editor = new ExpenseEditor(orderId, db, this);
        if (editor->exec() == QDialog::Accepted) {
            refreshTable();
        }
        delete editor;
    });
}

ExpensePage::~ExpensePage()
{
    delete ui;
    delete expenseModel;
}

void ExpensePage::setupExpenseTable()
{
    if (!expenseModel->select()) {
        QMessageBox::warning(this, "Data Error", "Failed to load expenses: " + expenseModel->lastError().text());
        return;
    }
    ui->expenseTable->setModel(expenseModel);
    ui->expenseTable->setColumnHidden(10, true); // Hide Created At column (assuming col 10 exists)
    ui->expenseTable->resizeColumnsToContents();
    ui->expenseTable->horizontalHeader()->setStretchLastSection(false);
    ui->expenseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->expenseTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->expenseTable->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void ExpensePage::refreshTable()
{
    expenseModel->select(); // Force refresh
    ui->expenseTable->resizeColumnsToContents();
}

void ExpensePage::addExpense()
{
    ExpenseEditor editor("", db, this);  // Empty ID for new
    if (editor.exec() == QDialog::Accepted) {
        refreshTable();
    }
}

void ExpensePage::editExpense()
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
        refreshTable();
    }
}

void ExpensePage::deleteExpense()
{
    QModelIndexList selection = ui->expenseTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select an expense order to delete.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete the selected expense order(s)?",
        QMessageBox::Yes | QMessageBox::No
    );
    if (reply != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query(db);
    bool success = true;

    // Sort descending to handle multi-select safely (though not removing rows, just for consistency)
    std::sort(selection.begin(), selection.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() > b.row();
    });

    for (const auto &index : selection) {
        QString orderId = expenseModel->data(expenseModel->index(index.row(), 0)).toString();
        query.prepare("DELETE FROM expense_orders WHERE order_id = :orderId");
        query.bindValue(":orderId", orderId);
        if (!query.exec()) {
            success = false;
            QMessageBox::warning(this, "Database Error", "Failed to delete order: " + query.lastError().text());
            break;
        }
    }

    if (success) {
        refreshTable();
    }
}