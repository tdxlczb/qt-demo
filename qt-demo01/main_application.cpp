#include "main_application.h"
#include "main_widget.h"

MainApplication::MainApplication(int &argc, char **argv)
    : QApplication(argc,argv)
{
    mainWidget_ = new MainWidget();
}


MainApplication::~MainApplication()
{

}
