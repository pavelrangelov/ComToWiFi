#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>
#include <QStyleFactory>
#include <QDebug>
#include "maindialog.h"

#include "settings.h"

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QSharedMemory mem;
    mem.setNativeKey(APP_NAME);

    if (mem.attach()) {
        mem.detach();
        QMessageBox::information(NULL, NULL, "Application is already running\nin the system tray.");
        return -1;
    }

    mem.create(32);

    a.setApplicationName(APP_NAME);
    a.setApplicationVersion(APP_VERS);
    a.setOrganizationName(ORG_NAME);

    MainDialog w;
    w.show();

    return a.exec();
}
