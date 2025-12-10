#include <QApplication>
#include <QSerialPort>
#include <QDebug>

#include "mythread.h"
#include "settings.h"

//-----------------------------------------------------------------------------
MyThread::MyThread(QObject *parent) : QThread(parent) {
    m_connectTimer = new QTimer(this);
    m_connectTimer->setSingleShot(true);
    QObject::connect(m_connectTimer, &QTimer::timeout, this, &MyThread::checkForConnected);

    #ifdef USE_CONTROL_SOCKET
    m_pingTimer = new QTimer(this);
    m_pingTimer->setSingleShot(false);
    QObject::connect(m_pingTimer, &QTimer::timeout, this, &MyThread::sendPing);
    m_pingCounter = 0;
    #endif
}

//-----------------------------------------------------------------------------
MyThread::~MyThread() {

}

//-----------------------------------------------------------------------------
void MyThread::run() {
    for (;;) {
        msleep(5000);
    }
}

//-----------------------------------------------------------------------------
QString MyThread::createSerialPortName(QString name) {
    QString fname = "";
    #ifdef Q_OS_WIN
    int index = name.indexOf("(COM");
    if (index != -1) {
        index++;
        name = name.mid(index,5);
        name.remove(')');
    }
    fname = "\\\\.\\";
    fname += name;
    #else
    fname = "/dev/";
    fname += name;
    #endif
    return fname;
}

//-----------------------------------------------------------------------------
bool MyThread::serialConnect() {
    m_virtualSerialPort = new QSerialPort(this);
    m_virtualSerialPort->setPortName(createSerialPortName(m_virtualSerialPortName));
    m_virtualSerialPort->setBaudRate(QSerialPort::Baud115200);
    m_virtualSerialPort->setDataBits(QSerialPort::Data8);
    m_virtualSerialPort->setStopBits(QSerialPort::OneStop);
    m_virtualSerialPort->setParity(QSerialPort::NoParity);
    m_virtualSerialPort->setFlowControl(QSerialPort::NoFlowControl);
    m_virtualSerialPort->close();

    if (!m_virtualSerialPort->open(QIODevice::ReadWrite)) {
        return false;
    }

    QObject::connect(m_virtualSerialPort, &QIODevice::readyRead, this, &MyThread::serialDataAvailable);

    return true;
}

//-----------------------------------------------------------------------------
void MyThread::serialDisconnect() {
    if (m_virtualSerialPort) {
        m_virtualSerialPort->close();
    }
}

//-----------------------------------------------------------------------------
void MyThread::tcpConnect() {
    m_txSocket = new QTcpSocket(this);
    QObject::connect(m_txSocket, &QTcpSocket::connected, this, &MyThread::txSocketConnected);
    QObject::connect(m_txSocket, &QTcpSocket::disconnected, this, &MyThread::txSocketDisconnected);
    QObject::connect(m_txSocket, &QAbstractSocket::errorOccurred, this, &MyThread::txSocketError);
    QObject::connect(m_txSocket, &QAbstractSocket::readyRead, this, &MyThread::txReadData);
    m_txSocket->abort();
    m_txSocket->connectToHost(m_host, ESP_RXPORT); // We will send data to ESP32 Rx TCP Port

    m_rxSocket = new QTcpSocket(this);
    QObject::connect(m_rxSocket, &QTcpSocket::connected, this, &MyThread::rxSocketConnected);
    QObject::connect(m_rxSocket, &QTcpSocket::disconnected, this, &MyThread::rxSocketDisconnected);
    QObject::connect(m_rxSocket, &QAbstractSocket::errorOccurred, this, &MyThread::rxSocketError);
    QObject::connect(m_rxSocket, &QAbstractSocket::readyRead, this, &MyThread::rxReadData);
    m_rxSocket->abort();
    m_rxSocket->connectToHost(m_host, ESP_TXPORT); // We will receive data from ESP Tx TCP Port

    #ifdef USE_CONTROL_SOCKET
    m_ctSocket = new QTcpSocket(this);
    QObject::connect(m_ctSocket, &QTcpSocket::connected, this, &MyThread::ctSocketConnected);
    QObject::connect(m_ctSocket, &QTcpSocket::disconnected, this, &MyThread::ctSocketDisconnected);
    QObject::connect(m_ctSocket, &QAbstractSocket::errorOccurred, this, &MyThread::ctSocketError);
    QObject::connect(m_ctSocket, &QAbstractSocket::readyRead, this, &MyThread::ctReadData);
    m_ctSocket->abort();
    m_ctSocket->connectToHost(m_host, ESP_CTPORT);
    #endif

    m_connectTimer->start(5000);
}

//-----------------------------------------------------------------------------
void MyThread::tcpDisconnect() {
    if (m_txSocket) {
        m_txSocket->abort();
        m_txSocket->close();
        m_txSocket->deleteLater();
    }
    if (m_rxSocket) {
        m_rxSocket->abort();
        m_rxSocket->close();
        m_rxSocket->deleteLater();
    }
    #ifdef USE_CONTROL_SOCKET
    if (m_ctSocket) {
        m_ctSocket->abort();
        m_ctSocket->close();
        m_ctSocket->deleteLater();
        m_pingTimer->stop();
    }
    #endif
}

//-----------------------------------------------------------------------------
void MyThread::checkForConnected() {
    if (!m_txSocket || !m_rxSocket) {
        emit connectError(tr("Failed to create sockets"));
        return;
    }
    if (m_txSocket->state() != QAbstractSocket::ConnectedState) {
        QString errstr = tr("Failed to connect to: %1:%2").arg(m_host).arg(ESP_TXPORT);
        emit connectError(errstr);
        return;
    }
    if (m_rxSocket->state() != QAbstractSocket::ConnectedState) {
        QString errstr = tr("Failed to connect to: %1:%2").arg(m_host).arg(ESP_RXPORT);
        emit connectError(errstr);
        return;
    }
    #ifdef USE_CONTROL_SOCKET
    if (m_ctSocket->state() != QAbstractSocket::ConnectedState) {
        QString errstr = tr("Failed to connect to: %1:%2").arg(m_host).arg(ESP_CTPORT);
        emit connectError(errstr);
        return;
    }
    #endif
    emit connectError("OK");
}

//-----------------------------------------------------------------------------
void MyThread::txSocketConnected() {
    if (m_txSocket->state() == QAbstractSocket::ConnectedState &&
        m_rxSocket->state() == QAbstractSocket::ConnectedState
        #ifdef USE_CONTROL_SOCKET
        && m_ctSocket->state() == QAbstractSocket::ConnectedState
        #endif
        ) {
        m_connectTimer->stop();
        emit connectError("OK");
    }
}

//-----------------------------------------------------------------------------
void MyThread::txSocketDisconnected() {
    emit connectError("TxDisconnected");
}

//-----------------------------------------------------------------------------
void MyThread::txSocketError(QAbstractSocket::SocketError error) {
    qDebug() << error;
}

//-----------------------------------------------------------------------------
void MyThread::txReadData() {
    QByteArray data = m_txSocket->readAll();
    data.clear();
}

//-----------------------------------------------------------------------------
void MyThread::rxSocketConnected() {
    if (m_txSocket->state() == QAbstractSocket::ConnectedState &&
        m_rxSocket->state() == QAbstractSocket::ConnectedState
        #ifdef USE_CONTROL_SOCKET
        && m_ctSocket->state() == QAbstractSocket::ConnectedState
        #endif
        ) {
        m_connectTimer->stop();
        emit connectError("OK");
    }
}

//-----------------------------------------------------------------------------
void MyThread::rxSocketDisconnected() {
    emit connectError("RxDisconnected");
}

//-----------------------------------------------------------------------------
void MyThread::rxSocketError(QAbstractSocket::SocketError error) {
    qDebug() << error;
}

//-----------------------------------------------------------------------------
void MyThread::rxReadData() {
    QByteArray bytes = m_rxSocket->readAll();
    m_virtualSerialPort->write(bytes);
}

#ifdef USE_CONTROL_SOCKET
//-----------------------------------------------------------------------------
void MyThread::ctSocketConnected() {
    if (m_txSocket->state() == QAbstractSocket::ConnectedState &&
        m_rxSocket->state() == QAbstractSocket::ConnectedState &&
        m_ctSocket->state() == QAbstractSocket::ConnectedState) {
        m_connectTimer->stop();
        m_pingTimer->start(PING_TIME);
        m_pingCounter = 0;
        emit connectError("OK");
    }
}

//-----------------------------------------------------------------------------
void MyThread::ctSocketDisconnected() {
    emit connectError("CtDisconnected");
}

//-----------------------------------------------------------------------------
void MyThread::ctSocketError(QAbstractSocket::SocketError error) {
    qDebug() << error;
}

//-----------------------------------------------------------------------------
void MyThread::ctReadData() {
    QByteArray bytes = m_ctSocket->readAll();
    if (bytes.length() && bytes.indexOf("pong") != -1) {
        //qDebug() << "Got pong";
        m_pingCounter = 0;
    }
}

const char *cmd_ping = "{\"cmd\":\"ping\"}\n";
//-----------------------------------------------------------------------------
void MyThread::sendPing() {
    if (m_ctSocket->isValid() && m_ctSocket->isOpen()) {
        m_ctSocket->write(cmd_ping);
        if (++m_pingCounter >= PING_TOUT_COUNT) {
            emit connectError("CtDisconnected");
            doDisconnect();
        }
    }
}

#endif

//-----------------------------------------------------------------------------
void MyThread::serialDataAvailable() {
    QByteArray bytes = m_virtualSerialPort->readAll();
    m_txSocket->write(bytes);
}

//-----------------------------------------------------------------------------
void MyThread::doConnect(QString &hostName, QString &serialPortName) {
    m_host = hostName;
    m_virtualSerialPortName = serialPortName;

    if (!serialConnect()) {
        QString errstr = tr("Failed to open port: %1").arg(m_virtualSerialPortName);
        emit connectError(errstr);
        return;
    }

    tcpConnect();
}

//-----------------------------------------------------------------------------
void MyThread::doDisconnect() {
    serialDisconnect();
    tcpDisconnect();
}
