#include "displaysysinfo.h"
#include <QApplication>
#include <unistd.h>
#include <stdio.h>
int main(int argc, char *argv[])
{
//    printf("test 1\n");
    QApplication a(argc, argv);
//    printf("test 2\n");
    DisplaySysInfo w;
//    printf("test 3\n");
    w.show();
//    printf("test 4\n");
//    sleep(1);
    //return 0;
    a.exec();
    return 0;
}
