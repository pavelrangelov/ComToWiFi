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
bool MyThread::serialConnect(const QString &virtualSerialPortName,
                             const QSerialPort::BaudRate baudRate,
                             const QSerialPort::DataBits dataBits,
                             const QSerialPort::StopBits stopBits,
                             const QSerialPort::Parity parity,
                             const QSerialPort::FlowControl flowControl) {
    m_virtualSerialPortName = virtualSerialPortName;
    m_baudRate = baudRate;
    m_dataBits = dataBits;
    m_stopBits = stopBits;
    m_parity = parity;
    m_flowControl = flowControl;

    m_virtualSerialPort = new QSerialPort(this);
    /*
    m_virtualSerialPort->setPortName(createSerialPortName(m_virtualSerialPortName));
    m_virtualSerialPort->setBaudRate(m_baudRate);
    m_virtualSerialPort->setDataBits(m_dataBits);
    m_virtualSerialPort->setStopBits(m_stopBits);
    m_virtualSerialPort->setParity(m_parity);
    m_virtualSerialPort->setFlowControl(m_flowControl);
    m_virtualSerialPort->close();
    */
    qDebug() << m_virtualSerialPort->baudRate();
    qDebug() << m_virtualSerialPort->dataBits();
    qDebug() << m_virtualSerialPort->stopBits();
    qDebug() << m_virtualSerialPort->parity();
    qDebug() << m_virtualSerialPort->flowControl();

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
bool MyThread::tcpConnect(const QString &host, const int tcpTxPort, const int tcpRxPort) {
    m_host = host;
    m_tcpTxPort = tcpTxPort;
    m_tcpRxPort = tcpRxPort;

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
    m_txSocket->connectToHost(m_host, m_tcpTxPort);
    m_rxSocket->abort();
    m_rxSocket->connectToHost(m_host, m_tcpRxPort);

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
    qDebug() << "txSocketConnected()";
}

//-----------------------------------------------------------------------------
void MyThread::txSocketDisconnected() {
    qDebug() << "txSocketDisconnected()";
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
    qDebug() << "rxSocketConnected()";
}

//-----------------------------------------------------------------------------
void MyThread::rxSocketDisconnected() {
    qDebug() << "rxSocketDisconnected()";
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
