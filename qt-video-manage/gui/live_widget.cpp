#include "live_widget.h"
#include "ui_live_widget.h"

LiveWidget::LiveWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LiveWidget)
{
    ui->setupUi(this);
}

LiveWidget::~LiveWidget()
{
    delete ui;
}
