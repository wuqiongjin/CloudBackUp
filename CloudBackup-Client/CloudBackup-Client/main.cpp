#include "QtCloudBackupClient.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtCloudBackupClient w;
    w.show();
    return a.exec();
}