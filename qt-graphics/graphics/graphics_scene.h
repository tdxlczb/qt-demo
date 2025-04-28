#ifndef GRAPHICS_SCENE_H
#define GRAPHICS_SCENE_H

#include <QGraphicsScene>

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    GraphicsScene(QObject *parent = nullptr);
    ~GraphicsScene();
signals:
};

#endif // GRAPHICS_SCENE_H
