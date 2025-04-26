#include "main_window.h"
#include "ui_main_window.h"
#include "main_widget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_pMainWidget = new MainWidget(nullptr);
    m_pMainWidget->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

