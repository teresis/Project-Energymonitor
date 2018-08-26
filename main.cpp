#include "displaysysinfo.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DisplaySysInfo w;
    w.show();
    printf("test");
    return a.exec();
}
