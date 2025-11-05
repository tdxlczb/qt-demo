#ifndef VIDEO_MANAGER_H
#define VIDEO_MANAGER_H

#include <QWidget>
#include <QGridLayout>

namespace Ui {
class VideoManager;
}

class VideoManager : public QWidget
{
    Q_OBJECT

public:
    explicit VideoManager(QWidget *parent = nullptr);
    ~VideoManager();

private:
    Ui::VideoManager *ui;
    QGridLayout* m_pGridLayout = nullptr;
};

#endif // VIDEO_MANAGER_H
