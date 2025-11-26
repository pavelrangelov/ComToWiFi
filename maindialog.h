#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "mythread.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainDialog;
}
QT_END_NAMESPACE

class MainDialog : public QDialog {
    Q_OBJECT

    public:
        MainDialog(QWidget *parent = nullptr);
        ~MainDialog();

    private:
        Ui::MainDialog *ui;
        MyThread *m_thread;
        QString m_tcpTxPort;
        QString m_tcpRxPort;
        bool m_connected;
        QSystemTrayIcon *m_trayIcon;
        QMenu *m_trayIconMenu;
        QAction *m_actionRestore;
        QAction *m_actionQuit;

        QStringList getPortNames();
        void saveSettings();
        void loadSettings();
        void setConnected(bool connected);
        void createTrayIcon();
        void quitApplication();

    protected:
        virtual void closeEvent(QCloseEvent *ev);

    private slots:
        void connectToServer();
};
#endif // MAINDIALOG_H
