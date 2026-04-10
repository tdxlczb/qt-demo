#ifndef FLOATING_TOOLBAR_H
#define FLOATING_TOOLBAR_H

#include <QWidget>
#include <QGridLayout>

class FloatingToolbar : public QWidget
{
    Q_OBJECT
public:
    explicit FloatingToolbar(QWidget* parent = nullptr);
    ~FloatingToolbar();
signals:

private:

};


class FloatingToolbarTest : public QWidget
{
    Q_OBJECT
public:
    explicit FloatingToolbarTest(QWidget* parent = nullptr);
    ~FloatingToolbarTest();
signals:

private:

};


#endif // FLOATING_TOOLBAR_H
