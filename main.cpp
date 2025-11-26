#include <QApplication>
#include "maindialog.h"

#include "settings.h"

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    a.setApplicationName(APP_NAME);
    a.setApplicationVersion(APP_VERS);
    a.setOrganizationName(ORG_NAME);

    MainDialog w;
    w.show();

    return a.exec();
}
