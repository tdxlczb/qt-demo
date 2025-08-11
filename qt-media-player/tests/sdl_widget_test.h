#ifndef SDL_WIDGET_H
#define SDL_WIDGET_H

#include <QWidget>
#include <QTimer>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

class SDLWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SDLWidget(QWidget* parent = nullptr);
    ~SDLWidget();

    bool initSDL();
    void renderFrame();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void cleanupSDL();
private:
    SDL_Window* m_sdlWindow = nullptr;
    SDL_Renderer* m_sdlRenderer = nullptr;
    bool m_sdlInitialized = false;
    QTimer* m_timer = nullptr;

};

int sdl_test_main(int argc, char* argv[]);

#endif // SDL_WIDGET_H
