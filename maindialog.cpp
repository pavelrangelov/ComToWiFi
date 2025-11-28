#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>

#include "maindialog.h"
#include "ui_maindialog.h"
#include "settings.h"

#define DEFAULT_HOST        "192.168.4.1"
#define DEFAULT_TCPTXPORT   "8890"
#define DEFAULT_TCPRXPORT   "8891"

//-----------------------------------------------------------------------------
MainDialog::MainDialog(QWidget *parent) : QDialog(parent), ui(new Ui::MainDialog) {
    ui->setupUi(this);

    setWindowTitle(APP_NAME);

    ui->comboVirtualPort->addItems(getPortNames());

    createTrayIcon();

    loadSettings();

    connect(ui->btnConnect, &QPushButton::clicked, this, &MainDialog::connectToServer);
    connect(ui->btnDefault, &QPushButton::clicked, this, &MainDialog::resetToDefault);

    m_thread = new MyThread(this);
    m_thread->start();

    setConnected(false);
}

//-----------------------------------------------------------------------------
MainDialog::~MainDialog() {
    if (m_thread) {
        m_thread->terminate();
        m_thread->wait();
    }
    delete ui;
}

//-----------------------------------------------------------------------------
void MainDialog::closeEvent(QCloseEvent *ev) {
    if (!ev->spontaneous() || !isVisible()) {
        return;
    }

    if (m_trayIcon->isVisible()) {
        hide();
        ev->ignore();
        return;
    }

    ev->accept();
}

//-----------------------------------------------------------------------------
void MainDialog::quitApplication() {
    saveSettings();
    qApp->quit();
}

//-----------------------------------------------------------------------------
QStringList MainDialog::getPortNames() {
    QStringList list;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (info.description().isEmpty()) {
            list.append(QString("%1").arg(info.portName()));
        } else {
            list.append(QString("%1 (%2)").arg(info.portName(), info.description()));
        }
    }

    return list;
}

//-----------------------------------------------------------------------------
QString MainDialog::parsePort(const QString &text) {
    QString portName = text;

    int i = text.indexOf(QChar(0x20));

    if (i != -1) {
        portName = text.left(i);
    }

    return portName;
}

//-----------------------------------------------------------------------------
void MainDialog::saveSettings() {
    QSettings settings;
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("VirtualPort", parsePort(ui->comboVirtualPort->currentText()));
    settings.setValue("Host", ui->editHost->text());
    settings.setValue("TcpTxPort", ui->editTcpTxPort->text());
    settings.setValue("TcpRxPort", ui->editTcpRxPort->text());
}

//-----------------------------------------------------------------------------
void MainDialog::loadSettings() {
    QSettings settings;
    restoreGeometry(settings.value("Geometry").toByteArray());
    ui->comboVirtualPort->setCurrentText(settings.value("VirtualPort", "").toString());
    ui->editHost->setText(settings.value("Host", DEFAULT_HOST).toString());
    ui->editTcpTxPort->setText(settings.value("TcpTxPort", DEFAULT_TCPTXPORT).toString());
    ui->editTcpRxPort->setText(settings.value("TcpRxPort", DEFAULT_TCPRXPORT).toString());
}

//-----------------------------------------------------------------------------
void MainDialog::connectToServer() {
    if (!m_connected) {
        m_connected = true;
        setConnected(true);
    } else {
        m_connected = false;
        setConnected(false);
    }
}

//-----------------------------------------------------------------------------
void MainDialog::setConnected(bool connected) {
    m_connected = connected;
    if (m_connected) {
        if (!m_thread->serialConnect(parsePort(ui->comboVirtualPort->currentText()))) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to connect to port: %1").arg(ui->comboVirtualPort->currentText()));
            return;
        }
        ui->btnConnect->setText(tr("Disconnect"));
        ui->btnConnect->setIcon(QIcon(":/images/stop-32.png"));
        ui->btnConnect->setIconSize(QSize(16,16));
        m_trayIcon->setIcon(QIcon(":/images/bulb_on.png"));
    } else {
        m_thread->serialDisconnect();
        ui->btnConnect->setText(tr("Connect"));
        ui->btnConnect->setIcon(QIcon(":/images/play-32.png"));
        ui->btnConnect->setIconSize(QSize(16,16));
        m_trayIcon->setIcon(QIcon(":/images/bulb_off.png"));
    }
}

//-----------------------------------------------------------------------------
void MainDialog::createTrayIcon() {
    m_actionRestore = new QAction(tr("&Restore"), this);
    QObject::connect(m_actionRestore, &QAction::triggered, this, &MainDialog::showNormal);
    m_actionQuit = new QAction(tr("&Quit"), this);
    QObject::connect(m_actionQuit, &QAction::triggered, this, &MainDialog::quitApplication);

    m_trayIconMenu = new QMenu(this);
    m_trayIconMenu->addAction(m_actionRestore);
    m_trayIconMenu->addAction(m_actionQuit);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_trayIconMenu);
    m_trayIcon->setIcon(QIcon(":/images/bulb_off.png"));
    m_trayIcon->show();
}

//-----------------------------------------------------------------------------
void MainDialog::resetToDefault() {
    ui->editHost->setText(DEFAULT_HOST);
    ui->editTcpTxPort->setText(DEFAULT_TCPTXPORT);
    ui->editTcpRxPort->setText(DEFAULT_TCPRXPORT);
}
