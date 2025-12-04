#include <QApplication>
#include <QSerialPort>
#include <QDebug>

#ifdef WIN32
#include <winsock2.h>
#include <ws2def.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#include "mythread.h"
#include "settings.h"

//-----------------------------------------------------------------------------
MyThread::MyThread(QObject *parent) : QThread(parent) {
    m_connectTimer = new QTimer(this);
    m_connectTimer->setSingleShot(true);
    QObject::connect(m_connectTimer, &QTimer::timeout, this, &MyThread::checkForConnected);
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

    m_rxSocket = new QTcpSocket(this);
    QObject::connect(m_rxSocket, &QTcpSocket::connected, this, &MyThread::rxSocketConnected);
    QObject::connect(m_rxSocket, &QTcpSocket::disconnected, this, &MyThread::rxSocketDisconnected);
    QObject::connect(m_rxSocket, &QAbstractSocket::errorOccurred, this, &MyThread::rxSocketError);
    QObject::connect(m_rxSocket, &QAbstractSocket::readyRead, this, &MyThread::rxReadData);

    m_txSocket->abort();
    m_txSocket->connectToHost(m_host, ESP_RXPORT); // We will send data to ESP32 Rx TCP Port

    m_rxSocket->abort();
    m_rxSocket->connectToHost(m_host, ESP_TXPORT); // We will receive data from ESP Tx TCP Port

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
        qDebug() << "3";
        QString errstr = tr("Failed to connect to: %1:%2").arg(m_host).arg(ESP_RXPORT);
        emit connectError(errstr);
        return;
    }
    emit connectError("OK");
}

//-----------------------------------------------------------------------------
void MyThread::txSocketConnected() {
    if (m_txSocket->state() == QAbstractSocket::ConnectedState &&
       (m_rxSocket->state() == QAbstractSocket::ConnectedState)) {
        m_connectTimer->stop();
        setKeepAlive(m_txSocket);
        setKeepAlive(m_rxSocket);
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
        (m_rxSocket->state() == QAbstractSocket::ConnectedState)) {
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

//-----------------------------------------------------------------------------
bool MyThread::setKeepAlive(QTcpSocket *socket) {
    qintptr s = socket->socketDescriptor();
    if (s == -1) {
        return false;
    }

    #ifdef WIN32
    struct tcp_keepalive alive;
    DWORD ret = 0;

    alive.onoff = 1;
    alive.keepalivetime = 10000;
    alive.keepaliveinterval = 2000;

    if (WSAIoctl(s, SIO_KEEPALIVE_VALS, &alive, sizeof(alive), NULL, 0, &ret, NULL, NULL) == SOCKET_ERROR) {
        qDebug() << "WSAIoctl: error";
        return false;
    }
    #else
    int enable = 1;
    int maxidle = 10;
    int interval = 2;
    int count = 3;

    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));
    setsockopt(s, IPPROTO_TCP, TCP_KEEPIDLE, &maxidle, sizeof(maxidle));
    setsockopt(s, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count));
    setsockopt(s, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
    #endif

    return true;
}
