#ifndef FUNCTION_WIDGET_H
#define FUNCTION_WIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QHash>

using Method = std::function<void()>;

class FunctionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FunctionWidget(QWidget* parent = nullptr);
    ~FunctionWidget();
signals:

private slots:
    // 按钮点击统一响应函数
    void onButtonClicked();

private:
    void InitFunctions();
    void AddFunction(QString name, Method method);
    void CreateGrid();
    void CreateEdit();
    QString GetEditText();

    void AttachWindow();

private:
    int m_rowCount = 3;
    int m_colCount = 3;
    int m_buttonCount = 7;

    QVBoxLayout* m_mainLayout;
    QTextEdit* m_infoTextEdit;

    QHash<QString, Method> m_hashMethods;
};


#endif // FUNCTION_WIDGET_H
