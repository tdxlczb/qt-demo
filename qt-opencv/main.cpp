#include "main_window.h"

#include <QApplication>
#include <opencv2/opencv.hpp>
#include "fisheye_correction/fisheye_correction_demo01.h"
#include "image_stitching/image_stitching_demo01.h"

//int main(int argc, char *argv[])
//{
//    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();
//    return a.exec();
//}


int main(int argc, char* argv[])
{
    //UnwrapCircularDemo();
    FisheyeDemo01();
    //ImageStitchingDemo02();
    return 0;
}
