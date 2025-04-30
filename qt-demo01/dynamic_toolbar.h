#ifndef DYNAMICTOOLBAR_H
#define DYNAMICTOOLBAR_H

#include <QWidget>
#include <QMenu>

class DynamicToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit DynamicToolBar(QWidget *parent = nullptr);
    ~DynamicToolBar();
signals:

};

#endif // DYNAMICTOOLBAR_H
