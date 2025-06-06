#include "toast_widget.h"

ToastWidget::ToastWidget(QWidget *parent)
    : QWidget{parent}
{}


#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QPropertyAnimation>

void showToast(QWidget* parent, const QString& message, int timeout = 2000) {
    QLabel* toast = new QLabel(parent);
    toast->setText(message);
    toast->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(0, 0, 0, 180);"
        "   color: white;"
        "   padding: 10px;"
        "   border-radius: 5px;"
        "}"
    );
    toast->setAlignment(Qt::AlignCenter);
    toast->setAttribute(Qt::WA_DeleteOnClose); // 自动销毁
    // 关键修复：启用透明背景和窗口属性，不设置窗口属性动画不生效
    //toast->setAttribute(Qt::WA_TranslucentBackground);
    toast->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    // 设置 Toast 的位置（居中）
    toast->adjustSize();
    toast->move(
        (parent->width() - toast->width()) / 2,
        parent->height() - toast->height() - 20
    );
    toast->show();

    // 淡入动画（可选）
    QPropertyAnimation* fadeIn = new QPropertyAnimation(toast, "windowOpacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0);
    fadeIn->setEndValue(1);
    fadeIn->start();
    // 定时关闭 Toast
    QTimer::singleShot(timeout, [=]() {
        // 淡出动画（可选）
        QPropertyAnimation* fadeOut = new QPropertyAnimation(toast, "windowOpacity");
        fadeOut->setDuration(3000);
        fadeOut->setStartValue(1);
        fadeOut->setEndValue(0);
        fadeOut->start();
        QObject::connect(fadeOut, &QPropertyAnimation::finished, toast, &QLabel::close);
        });
}


#include <QGraphicsOpacityEffect>

void showToast2(QWidget* parent, const QString& message, int timeout = 2000) {
    QLabel* toast = new QLabel(parent);
    toast->setText(message);
    toast->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(0, 0, 0, 180);"
        "   color: white;"
        "   padding: 10px;"
        "   border-radius: 5px;"
        "}"
    );

    // 设置透明度动画
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(toast);
    toast->setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(0); // 初始透明

    toast->adjustSize();
    toast->move(
        (parent->width() - toast->width()) / 2,
        parent->height() - toast->height() - 20
    );
    toast->show();

    // 淡入动画
    QPropertyAnimation* fadeIn = new QPropertyAnimation(opacityEffect, "opacity");
    fadeIn->setDuration(1000);
    fadeIn->setStartValue(0);
    fadeIn->setEndValue(0.9);
    fadeIn->start();

    // 定时关闭 + 淡出
    QTimer::singleShot(timeout, [=]() {
        QPropertyAnimation* fadeOut = new QPropertyAnimation(opacityEffect, "opacity");
        fadeOut->setDuration(2000);
        fadeOut->setStartValue(0.9);
        fadeOut->setEndValue(0);
        fadeOut->start();

        QObject::connect(fadeOut, &QPropertyAnimation::finished, toast, &QLabel::close);
        });
}

int test(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    QGridLayout* layout = new QGridLayout(&window);

    QPushButton* button = new QPushButton("Show Toast");
    layout->addWidget(button);

    QObject::connect(button, &QPushButton::clicked, [&]() {
        showToast(&window, "This is a Toast message!");
        });

    window.resize(400, 300);
    window.show();
    return app.exec();
}