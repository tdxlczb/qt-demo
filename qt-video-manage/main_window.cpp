#include "main_window.h"
#include "ui_main_window.h"
#include "gui/main_widget.h"
#include "gui/media_manager_widget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // m_pMainWidget = new MainWidget(nullptr);
    // m_pMainWidget->show();

    m_pManagerWidget = new MediaManagerWidget(nullptr);
    m_pManagerWidget->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

