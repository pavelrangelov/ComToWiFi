#include <QApplication>
#include <QSerialPort>
#include <QDebug>
#include "mythread.h"

//-----------------------------------------------------------------------------
MyThread::MyThread(QObject *parent) : QThread(parent) {
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
bool MyThread::serialConnect(const QString &virtualSerialPortName) {
    m_virtualSerialPortName = virtualSerialPortName;

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
bool MyThread::tcpConnect(const QString &host, const int espTxPort, const int espRxPort) {
    m_host = host;
    m_espTxPort = espTxPort;
    m_espRxPort = espRxPort;

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
    m_txSocket->connectToHost(m_host, m_espRxPort); // We will send data to ESP32 Rx TCP Port
    if (!m_txSocket->waitForConnected(10000)) {
        return false;
    }

    m_rxSocket->abort();
    m_rxSocket->connectToHost(m_host, m_espTxPort); // We will receive data from ESP Tx TCP Port
    if (!m_rxSocket->waitForConnected(10000)) {
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
void MyThread::tcpDisconnect() {
    if (m_txSocket) {
        m_txSocket->abort();
        m_txSocket->waitForDisconnected(500);
        delete m_txSocket;
    }
    if (m_rxSocket) {
        m_rxSocket->abort();
        m_rxSocket->waitForDisconnected(500);
        delete m_rxSocket;
    }
}

//-----------------------------------------------------------------------------
void MyThread::txSocketConnected() {
    //qDebug() << "txSocketConnected()";
}

//-----------------------------------------------------------------------------
void MyThread::txSocketDisconnected() {
    //qDebug() << "txSocketDisconnected()";
}

//-----------------------------------------------------------------------------
void MyThread::txSocketError(QAbstractSocket::SocketError error) {
    qDebug() << "txSocketError: " << error;
}

//-----------------------------------------------------------------------------
void MyThread::txReadData() {
    // Do nothing
}

//-----------------------------------------------------------------------------
void MyThread::rxSocketConnected() {
    //qDebug() << "rxSocketConnected()";
}

//-----------------------------------------------------------------------------
void MyThread::rxSocketDisconnected() {
    //qDebug() << "rxSocketDisconnected()";
}

//-----------------------------------------------------------------------------
void MyThread::rxSocketError(QAbstractSocket::SocketError error) {
    qDebug() << "rxSocketError: " << error;
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
