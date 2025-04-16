#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMovie>

namespace Ui {
class MainWindow;
}

class QMovie;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_horizontalSlider_valueChanged(int value);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_spinBox_valueChanged(int value);

    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;
    QMovie *movie;
};

#endif // MAINWINDOW_H
