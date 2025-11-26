#include <QApplication>
#include <QSharedMemory>
#include "maindialog.h"

#include "settings.h"

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    QSharedMemory mem;
    mem.setNativeKey(APP_NAME);

    if (!mem.attach()) {
        mem.create(32);
    } else {
        return -1;
    }

    QApplication a(argc, argv);

    a.setApplicationName(APP_NAME);
    a.setApplicationVersion(APP_VERS);
    a.setOrganizationName(ORG_NAME);

    MainDialog w;
    w.show();

    return a.exec();
}
