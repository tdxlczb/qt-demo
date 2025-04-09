
#include "main_window.h"
#include "ui_main_window.h"
#include <QMovie>
#include <QString>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    int Imagelines;
    ui->setupUi(this);

    ui->label->setAlignment(Qt::AlignCenter);               //在两个维度对齐
    ui->label->setBackgroundRole(QPalette::Dark);          //label的黑影
    ui->label->setAutoFillBackground(true);                   //窗口小部件背景是否自动填充

    movie = new QMovie(this);                                           //实例movie
    movie->setFileName(":/resource/image/loading2.gif");                                     //设置gif路径

    movie->setCacheMode(QMovie::CacheAll);                  //缓存全部帧

    QSize size = ui->label->size();                                     //保存label的大小（调整后的）
    movie->setScaledSize(size);                                         //将缩放后的帧大小设置为大小
    ui->label->setMovie(movie);


    ui->horizontalSlider->setMinimum(0);                                      //滑块的最小值
    ui->horizontalSlider->setMaximum(movie->frameCount());      //滑块的最大值（影片的帧数）
    connect(movie, SIGNAL(frameChanged(int)),
            ui->horizontalSlider,SLOT(setValue(int)));                        //电影帧数改变值传递给滑块
    Imagelines = movie->frameCount();                                          //返回影片中的帧数
    qDebug() << "ImageLines" << Imagelines;                              //在调试窗口输出电影帧数

}

MainWindow::~MainWindow()
{
    delete ui;
}

/* start buttons */
void MainWindow::on_pushButton_clicked()
{
    movie->start();                 //运行
}

/* stop buttons */
void MainWindow::on_pushButton_2_clicked()
{
    movie->stop();                  //停止播放电影
}

/* save pix */
void MainWindow::on_pushButton_3_clicked()
{
    int id = movie->currentFrameNumber();           //返回当前帧的序列号。 电影中第一帧的编号是0
    QPixmap pix = movie->currentPixmap();           //将当前帧作为Q像素图返回
    pix.save(QString("./%1.png").arg(id));              //将id的值替换%1,pix保存的文件名为./%1.png
    qDebug() << "save image";
}

void MainWindow::on_spinBox_valueChanged(int value)
{
    movie->setSpeed(value);                     //调整当前速度
}



void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    movie->jumpToFrame(value);                  //跳转到帧号
}

void MainWindow::on_pushButton_4_clicked()
{
    movie->setPaused(1);                            //暂停播放
    qDebug() << "stop is press!";
}
