#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>

#include "maindialog.h"
#include "ui_maindialog.h"
#include "settings.h"

#define DEFAULT_HOST        "192.168.4.1"
#define DEFAULT_ESPTXPORT   8890
#define DEFAULT_ESPRXPORT   8891

//-----------------------------------------------------------------------------
MainDialog::MainDialog(QWidget *parent) : QDialog(parent), ui(new Ui::MainDialog) {
    ui->setupUi(this);

    setWindowTitle(APP_NAME);
    setWindowIcon(QIcon(":/images/connect-48.png"));

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
void MainDialog::keyPressEvent(QKeyEvent *ev) {
    if (ev->key() == Qt::Key_Escape) {
        ev->accept();
        return;
    }
    QDialog::keyPressEvent(ev);
}

//-----------------------------------------------------------------------------
void MainDialog::closeEvent(QCloseEvent *ev) {
    //if (!ev->spontaneous() || !isVisible()) {
    //    return;
    //}

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
    settings.setValue("TcpTxPort", ui->spinEspTxPort->value());
    settings.setValue("TcpRxPort", ui->spinEspRxPort->value());
}

//-----------------------------------------------------------------------------
void MainDialog::loadSettings() {
    QSettings settings;
    restoreGeometry(settings.value("Geometry").toByteArray());
    ui->comboVirtualPort->setCurrentText(settings.value("VirtualPort", "").toString());
    ui->editHost->setText(settings.value("Host", DEFAULT_HOST).toString());
    ui->spinEspTxPort->setValue(settings.value("TcpTxPort", DEFAULT_ESPTXPORT).toUInt());
    ui->spinEspRxPort->setValue(settings.value("TcpRxPort", DEFAULT_ESPRXPORT).toUInt());
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
            QMessageBox::critical(this, tr("Error"), tr("Failed to open port: %1").arg(ui->comboVirtualPort->currentText()));
            return;
        }
        if( !m_thread->tcpConnect(ui->editHost->text(), ui->spinEspTxPort->value(), ui->spinEspRxPort->value())) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to connect to: %1:%2").arg(ui->editHost->text(), ui->spinEspTxPort->value()));
            return;
        }
        ui->btnConnect->setText(tr("Disconnect"));
        ui->btnConnect->setIcon(QIcon(":/images/stop-32.png"));
        ui->btnConnect->setIconSize(QSize(16,16));
        m_trayIcon->setIcon(QIcon(":/images/bulb_on.png"));
    } else {
        m_thread->tcpDisconnect();
        m_thread->serialDisconnect();
        ui->btnConnect->setText(tr("Connect"));
        ui->btnConnect->setIcon(QIcon(":/images/play-32.png"));
        ui->btnConnect->setIconSize(QSize(16,16));
        m_trayIcon->setIcon(QIcon(":/images/bulb_off.png"));
    }
}

//-----------------------------------------------------------------------------
void MainDialog::createTrayIcon() {
    m_actionRestore = new QAction(QIcon(":/images/restore.png"), tr("&Restore"), this);
    QObject::connect(m_actionRestore, &QAction::triggered, this, &MainDialog::showNormal);
    m_actionQuit = new QAction(QIcon(":/images/exit.png"), tr("&Quit"), this);
    QObject::connect(m_actionQuit, &QAction::triggered, this, &MainDialog::quitApplication);

    m_trayIconMenu = new QMenu(this);
    m_trayIconMenu->addAction(m_actionRestore);
    m_trayIconMenu->addAction(m_actionQuit);

    m_trayIcon = new QSystemTrayIcon(this);
    QObject::connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainDialog::showNormal);
    m_trayIcon->setContextMenu(m_trayIconMenu);
    m_trayIcon->setIcon(QIcon(":/images/bulb_off.png"));
    m_trayIcon->show();
}

//-----------------------------------------------------------------------------
void MainDialog::resetToDefault() {
    ui->editHost->setText(DEFAULT_HOST);
    ui->spinEspTxPort->setValue(DEFAULT_ESPTXPORT);
    ui->spinEspRxPort->setValue(DEFAULT_ESPRXPORT);
}
