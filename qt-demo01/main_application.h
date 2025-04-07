#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QApplication>

class MainWidget;

class MainApplication : public QApplication
{
    Q_OBJECT
public:
    MainApplication(int &argc, char **argv);
   ~MainApplication();

signals:

private:
    MainWidget * mainWidget_;
};

#endif // MAINAPPLICATION_H
