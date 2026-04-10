#include "monitor_widget.h"
#include <QPainter>
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include "play_widget.h"

MonitorWidget::MonitorWidget(QWidget* parent) : QWidget(parent)
{
    //parentWidget = new QWidget();

    this->resize(600, 600);

    //parentWidget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    //parentWidget->setWindowOpacity(0.99);

    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint /*| Qt::WindowStaysOnTopHint*/);
    this->setMinimumSize(50, 50);

    //this->setAttribute(Qt::WA_TranslucentBackground);
    //this->setStyleSheet("background-color: rgba(0, 0, 0, 10);");

    m_gradLayout = new QGridLayout(this);
    m_gradLayout->setSpacing(2);
    m_gradLayout->setContentsMargins(2, 2, 2, 2);
    this->setLayout(m_gradLayout);

    SplitScreen(3, 3);
}

MonitorWidget::~MonitorWidget()
{
    qDebug() << "delete MonitorWidget";
}

void MonitorWidget::SplitScreen(int rows, int cols)
{
    int index = 0;
    for (size_t row = 0; row < rows; row++)
    {
        for (size_t col = 0; col < cols; col++)
        {
            auto playWidget = new PlayWidget(this, index);
            m_gradLayout->addWidget(playWidget, row, col);
            playWidget->show();
            m_playWidgetList.append(playWidget);
            index++;
        }
    }

}

