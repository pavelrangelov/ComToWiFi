#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
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

    ui->comboBaudrate->addItem("1200", QVariant(QSerialPort::Baud1200));
    ui->comboBaudrate->addItem("2400", QVariant(QSerialPort::Baud2400));
    ui->comboBaudrate->addItem("4800", QVariant(QSerialPort::Baud4800));
    ui->comboBaudrate->addItem("9600", QVariant(QSerialPort::Baud9600));
    ui->comboBaudrate->addItem("19200", QVariant(QSerialPort::Baud19200));
    ui->comboBaudrate->addItem("38400", QVariant(QSerialPort::Baud38400));
    ui->comboBaudrate->addItem("57600", QVariant(QSerialPort::Baud57600));
    ui->comboBaudrate->addItem("115200", QVariant(QSerialPort::Baud115200));

    ui->comboDataBits->addItem("5", QVariant(QSerialPort::Data5));
    ui->comboDataBits->addItem("6", QVariant(QSerialPort::Data6));
    ui->comboDataBits->addItem("7", QVariant(QSerialPort::Data7));
    ui->comboDataBits->addItem("8", QVariant(QSerialPort::Data8));

    ui->comboStopBits->addItem("1", QVariant(QSerialPort::OneStop));
    ui->comboStopBits->addItem("1.5", QVariant(QSerialPort::OneAndHalfStop));
    ui->comboStopBits->addItem("2", QVariant(QSerialPort::TwoStop));

    ui->comboParity->addItem("None", QVariant(QSerialPort::NoParity));
    ui->comboParity->addItem("Even", QVariant(QSerialPort::EvenParity));
    ui->comboParity->addItem("Odd", QVariant(QSerialPort::OddParity));

    ui->comboFlowControl->addItem("None", QVariant(QSerialPort::NoFlowControl));
    ui->comboFlowControl->addItem("Hardware", QVariant(QSerialPort::HardwareControl));
    ui->comboFlowControl->addItem("Software", QVariant(QSerialPort::SoftwareControl));

    createTrayIcon();

    setConnected(false);

    loadSettings();

    connect(ui->btnConnect, &QPushButton::clicked, this, &MainDialog::connectToServer);

    m_thread = new MyThread(this);
    m_thread->start();
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
void MainDialog::saveSettings() {
    QSettings settings;
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("BaudRate", ui->comboBaudrate->currentData());
    settings.setValue("DataBits", ui->comboDataBits->currentData());
    settings.setValue("StopBits", ui->comboStopBits->currentData());
    settings.setValue("Parity", ui->comboParity->currentData());
    settings.setValue("FlowControl", ui->comboFlowControl->currentData());
    settings.setValue("VirtualPort", ui->comboVirtualPort->currentText());
    settings.setValue("Host", ui->editHost->text());
    settings.setValue("TcpTxPort", ui->editTcpTxPort->text());
    settings.setValue("TcpRxPort", ui->editTcpRxPort->text());
}

//-----------------------------------------------------------------------------
void MainDialog::loadSettings() {
    QSettings settings;
    restoreGeometry(settings.value("Geometry").toByteArray());
    ui->comboBaudrate->setCurrentIndex(ui->comboBaudrate->findData(settings.value("BaudRate", QSerialPort::Baud9600).toUInt()));
    ui->comboDataBits->setCurrentIndex(ui->comboDataBits->findData(settings.value("DataBits", QSerialPort::Data8).toUInt()));
    ui->comboStopBits->setCurrentIndex(ui->comboStopBits->findData(settings.value("StopBits", QSerialPort::OneStop).toUInt()));
    ui->comboParity->setCurrentIndex(ui->comboParity->findData(settings.value("Parity", QSerialPort::NoParity).toUInt()));
    ui->comboFlowControl->setCurrentIndex(ui->comboFlowControl->findData(settings.value("FlowControl", QSerialPort::NoFlowControl).toUInt()));
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
        //m_thread->start();
    } else {
        m_connected = false;
        setConnected(false);
    }
}

//-----------------------------------------------------------------------------
void MainDialog::setConnected(bool connected) {
    m_connected = connected;
    if (m_connected) {
        ui->btnConnect->setText(tr("Disconnect"));
        ui->btnConnect->setIcon(QIcon(":/images/stop-32.png"));
        ui->btnConnect->setIconSize(QSize(16,16));
        m_trayIcon->setIcon(QIcon(":/images/bulb_on.png"));
    } else {
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
