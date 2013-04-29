#include "tar_backup.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    tar_backup w;
    w.show();
    
    return a.exec();
}
