#include "function_widget.h"
#include <QDebug>
#include <Windows.h>

#include "toolbar/floating_toolbar.h"
#include "video_monitor/attach_widget.h"
#include "video_monitor/monitor_widget.h"

FunctionWidget::FunctionWidget(QWidget* parent) : QWidget(parent)
{
    // 设置窗口标题
    this->setWindowTitle("功能窗口");
    this->resize(800, 400); // 窗口大小

    //this->setAttribute(Qt::WA_StyledBackground,true);//启用样式表背景
    //this->setStyleSheet("background-color: #FF9999;");

    this->setAutoFillBackground(true);//启用背景填充
    QPalette palette = this->palette();
    //通常指窗口部件的背景色
    palette.setColor(QPalette::Window, QColor(150, 150, 150));
    this->setPalette(palette);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    InitFunctions();

    CreateGrid();

    CreateEdit();
}

FunctionWidget::~FunctionWidget()
{
    qDebug() << "delete CustomChildWidget";
}

// 统一的按钮点击响应函数
void FunctionWidget::onButtonClicked()
{
    // 获取发送信号的按钮（知道点的是谁）
    QPushButton* clickedBtn = qobject_cast<QPushButton*>(sender());

    if (clickedBtn)
    {
        QString objName = clickedBtn->objectName();
        // 控制台打印
        qDebug() << "点击了：" << objName;
        if (m_hashMethods.contains(objName)) {
            m_hashMethods[objName]();
        }
    }
}

void FunctionWidget::InitFunctions()
{
    AddFunction("AttachWindow", [this]() {
        AttachWindow();
        });

    AddFunction("FloatingToolbar", [this]() {
        FloatingToolbarTest* toolbar = new FloatingToolbarTest(this);
        toolbar->show();
        });
    AddFunction("Test2", [this]() {
        qInfo() << "Test2";
        });
    AddFunction("Test3", [this]() {
        qInfo() << "Test3";
        });
    AddFunction("Test4", [this]() {
        qInfo() << "Test4";
        });
    AddFunction("Test5", [this]() {
        qInfo() << "Test5";
        });
    AddFunction("Test6", [this]() {
        qInfo() << "Test6";
        });
    AddFunction("Test7", [this]() {
        qInfo() << "Test7";
        });
}

void FunctionWidget::AddFunction(QString name, Method method)
{
    m_hashMethods.insert(name, method);
}

void FunctionWidget::CreateGrid()
{
    QWidget* widget = new QWidget(this);

    // 创建网格布局
    QGridLayout* gridLayout = new QGridLayout(widget);

    int index = 0;
    for (auto it = m_hashMethods.begin(); it != m_hashMethods.end(); ++it) {
        // 新建按钮
        QPushButton* btn = new QPushButton(this);

        // 设置按钮文字
        btn->setText(it.key());
        btn->setObjectName(it.key());

        // 设置按钮最小大小，更好看
        btn->setMinimumSize(70, 40);

        // 计算：当前按钮应该放在 第几行、第几列
        int row = index / m_rowCount;   // 行 = 索引 / 列数
        int col = index % m_colCount;   // 列 = 索引 % 列数

        // 添加到网格布局
        gridLayout->addWidget(btn, row, col);

        // 绑定点击信号（所有按钮共用一个槽函数）
        connect(btn, &QPushButton::clicked, this, &FunctionWidget::onButtonClicked);
        index++;
    }
    m_mainLayout->addWidget(widget);
}

void FunctionWidget::CreateEdit()
{
    QWidget* widget = new QWidget(this);
    widget->setMaximumHeight(100);

    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setSpacing(2);
    layout->setContentsMargins(2, 2, 2, 2);

    m_infoTextEdit = new QTextEdit(widget);
    m_infoTextEdit->setPlaceholderText("在此输入信息...");
    m_infoTextEdit->setMaximumHeight(30); // 限制高度
    m_infoTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 隐藏垂直滚动条

    QPushButton* btn = new QPushButton("button", widget);
    btn->resize(100, 50);

    layout->addWidget(m_infoTextEdit);
    layout->addWidget(btn);
    m_mainLayout->addWidget(widget);
}

QString FunctionWidget::GetEditText()
{
    return m_infoTextEdit->toPlainText();
}

void FunctionWidget::AttachWindow()
{
    int ihwnd = QString(GetEditText()).toInt();
    HWND parentHwnd = (HWND)(ihwnd);


    //HWND parentHwnd = (HWND)(132050);
    //HWND parentHwnd = (HWND)(40510956);

    auto* parentWidget = new AttachWidget2();
    //parentWidget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    //parentWidget->setWindowOpacity(0.99);
    //parentWidget->Attach(parentHwnd);

    //auto hWnd = (HWND)parentWidget->winId();
    //LONG_PTR ex = GetWindowLongW(hWnd, GWL_EXSTYLE);
    //SetWindowLongW(hWnd, GWL_EXSTYLE, ex | WS_EX_LAYERED);
    //ex = GetWindowLongW(hWnd, GWL_EXSTYLE);

    //auto ret = SetLayeredWindowAttributes(hWnd, 0, 204, LWA_ALPHA);
    //ret = SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    //parentWidget->show();
    //parentWidget->setAttribute(Qt::WA_StyledBackground, true);//启用样式表背景
    //parentWidget->setStyleSheet("background-color: #FF9999;");

    //parentWidget->setAttribute(Qt::WA_TranslucentBackground);
    //parentWidget->setStyleSheet("background-color: rgb(100, 100, 0);");

    auto pMonitor = new MonitorWidget();
    {
        HWND hwnd = (HWND)parentWidget->winId();
        //HWND parentHwnd = (HWND)(pAttachWiget->winId());
        if (!SetParent(hwnd, parentHwnd)) {
            qCritical() << QString("%1 SetParent %2 Error:").arg((qintptr)hwnd).arg((qintptr)parentHwnd) << GetLastError();
        }
    }

    {
        HWND hwnd = (HWND)pMonitor->winId();
        HWND parentHwnd = (HWND)(parentWidget->winId());
        if (!SetParent(hwnd, parentHwnd)) {
            qCritical() << QString("%1 SetParent %2 Error:").arg((qintptr)hwnd).arg((qintptr)parentHwnd) << GetLastError();
        }
    }
    pMonitor->move(10, 10);
    pMonitor->resize(600, 600);
    //pMonitor->setGeometry(0, 0, 600, 600);
    pMonitor->show();


    parentWidget->move(100, 100);
    parentWidget->resize(800, 800);
    //pMonitor->parentWidget.setGeometry(100, 100, 800, 800);
    parentWidget->show();

    //pMonitor->parentWidget.update();
    //pMonitor->parentWidget.raise();//不加这个会导致尺寸变更后不能点Widget
}

