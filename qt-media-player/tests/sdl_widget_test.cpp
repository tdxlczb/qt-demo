#include "sdl_widget_test.h"
#include <QDebug>
#include <QTimer>

SDLWidget::SDLWidget(QWidget* parent) : QWidget(parent)
{
    // 必须设置的属性
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_PaintOnScreen, true);  // 关键属性
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    // 禁用Qt的背景绘制
    setAutoFillBackground(false);
    // 禁用Qt的刷新，不能触发Qt的刷新，否则丢失和获取焦点时会一闪黑屏
    setUpdatesEnabled(false);
    // 设置焦点策略
    setFocusPolicy(Qt::StrongFocus);
}

SDLWidget::~SDLWidget()
{
    cleanupSDL();
}

bool SDLWidget::initSDL()
{
    if (m_sdlInitialized) return true;

    //SDL_LogSetOutputFunction([](void*, int, SDL_LogPriority, const char* msg) {
    //    // 自定义日志处理（或直接忽略）
    //    }, nullptr);
    //SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");  // 指定渲染驱动
    //SDL_SetHint(SDL_HINT_RENDER_LOGICAL_SIZE_MODE, "0");
    //SDL_LogSetPriority(SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_ERROR);  // 只输出错误

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        qDebug() << "SDL初始化失败:" << SDL_GetError();
        return false;
    }

    // 创建隐藏的SDL窗口
    m_sdlWindow = SDL_CreateWindowFrom((void*)this->winId());

    if (!m_sdlWindow) {
        qDebug() << "无法创建SDL窗口:" << SDL_GetError();
        return false;
    }

    // 创建渲染器
    m_sdlRenderer = SDL_CreateRenderer(m_sdlWindow, -1,
        SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC);
    if (!m_sdlRenderer) {
        qDebug() << "无法创建渲染器:" << SDL_GetError();
        return false;
    }

    m_sdlInitialized = true;
    
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this] () {
        //禁用Qt的刷新，只能用定时器主动刷新
        renderFrame();
        //update();
    });
    m_timer->start(40);
    return true;
}

void SDLWidget::cleanupSDL()
{
    if (m_sdlRenderer) {
        SDL_DestroyRenderer(m_sdlRenderer);
        m_sdlRenderer = nullptr;
    }
    if (m_sdlWindow) {
        SDL_DestroyWindow(m_sdlWindow);
        m_sdlWindow = nullptr;
    }
    SDL_Quit();
    m_sdlInitialized = false;
}

void SDLWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_sdlWindow) {
        // 延迟调整大小以避免竞争条件
        QTimer::singleShot(0, this, [this]() {
            SDL_SetWindowSize(m_sdlWindow, width(), height());
            // 立即渲染一帧
            renderFrame();
            //update();
            });
    }
}


void SDLWidget::paintEvent(QPaintEvent* event)
{
    renderFrame();
}

void SDLWidget::renderFrame()
{
    if (!m_sdlRenderer) return;

    // 1. 清除为背景色
    SDL_SetRenderDrawColor(m_sdlRenderer, 30, 30, 100, 255); // 深蓝色背景
    SDL_RenderClear(m_sdlRenderer);

    // 2. 绘制内容 - 一个随窗口大小变化的矩形
    SDL_Rect rect = {
        width() / 10, height() / 10,
        width() * 8 / 10, height() * 8 / 10
    };
    SDL_SetRenderDrawColor(m_sdlRenderer, 200, 200, 0, 255); // 黄色矩形
    SDL_RenderFillRect(m_sdlRenderer, &rect);

    // 3. 添加一些动态内容
    static int offset = 0;
    offset = (offset + 2) % (width() - 50);
    SDL_Rect movingRect = {
        offset, height() / 2 - 25,
        50, 50
    };
    SDL_SetRenderDrawColor(m_sdlRenderer, 255, 0, 0, 255); // 红色移动方块
    SDL_RenderFillRect(m_sdlRenderer, &movingRect);

    // 4. 提交渲染
    SDL_RenderPresent(m_sdlRenderer);

    // 5. 安排下一帧渲染
    //QTimer::singleShot(16, this, &SDLWidget::renderFrame); // ~60fps
}

#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include "tests/sdl_widget_test.h"

int sdl_test_main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QWidget mainWindow;
    QVBoxLayout layout(&mainWindow);

    SDLWidget* sdlWidget = new SDLWidget();
    layout.addWidget(sdlWidget);

    QPushButton* resizeButton = new QPushButton("Toggle Size");
    layout.addWidget(resizeButton);

    // 初始化SDL
    if (!sdlWidget->initSDL()) {
        return -1;
    }

    // 开始渲染
    sdlWidget->renderFrame();

    // 测试调整大小
    bool smallSize = false;
    QObject::connect(resizeButton, &QPushButton::clicked, [&]() {
        smallSize = !smallSize;
        sdlWidget->setFixedSize(smallSize ? QSize(400, 300) : QSize(800, 600));
        });

    mainWindow.resize(800, 600);
    mainWindow.show();

    return a.exec();
}