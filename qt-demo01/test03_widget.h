#ifndef TEST03_WIDGET_H
#define TEST03_WIDGET_H

#include <QWidget>
#include <QLabel>

namespace Ui {
class Test03Widget;
}

class AdaptiveLabel : public QLabel
{
    Q_OBJECT

public:
    explicit AdaptiveLabel(QWidget* parent = nullptr);
    ~AdaptiveLabel();

    void SetAdaptiveText(const QString& szText, bool bSetFixed = false);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QString m_szText;
};

class Test03Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Test03Widget(QWidget *parent = nullptr);
    ~Test03Widget();
private:
    void SetAdaptiveLabel(QLabel* label, const QString& szText);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    Ui::Test03Widget *ui;
};

#endif // TEST03_WIDGET_H
