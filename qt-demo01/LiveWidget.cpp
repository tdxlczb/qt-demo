#include "LiveWidget.h"
#include "LiveBottomFloatingBar.h"
#include <QHBoxLayout>

LiveWidget::LiveWidget(QWidget* parent) : QWidget(parent)
{
	this->resize(600, 450);
	QPalette palette;
	palette.setColor(QPalette::Window, QColor(50, 150, 150));
	// palette.setColor(QPalette::Background, Qt::black);//设置背景黑色
	this->setPalette(palette);
	this->setAutoFillBackground(true);//启用背景填充

	QHBoxLayout* layout = new QHBoxLayout(this);
	LiveBottomFloatingBar* pLBFB = new LiveBottomFloatingBar(this);
	//pLBFB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	pLBFB->show();
	layout->addWidget(pLBFB);
}

void LiveWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}
