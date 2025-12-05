#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QSerialPort>
#include <QTimer>

#include "settings.h"

class MyThread : public QThread {
    Q_OBJECT

    public:
        MyThread(QObject *parent = nullptr);
        ~MyThread();
        void tcpConnect();
        void tcpDisconnect();
        bool serialConnect();
        void serialDisconnect();

    private:
        QTcpSocket *m_txSocket = nullptr;
        QTcpSocket *m_rxSocket = nullptr;
        QTcpSocket *m_ctSocket = nullptr;
        QSerialPort *m_virtualSerialPort = nullptr;
        QString m_host = "";
        QString m_virtualSerialPortName = "";
        QTimer *m_connectTimer;
        QTimer *m_pingTimer;
        quint16 m_pingCounter;

        QString createSerialPortName(QString name);
        bool setKeepAlive(QTcpSocket *socket);

    protected:
        void run() override;

    private slots:
        void txSocketConnected();
        void txSocketDisconnected();
        void txSocketError(QAbstractSocket::SocketError error);
        void txReadData();

        void rxSocketConnected();
        void rxSocketDisconnected();
        void rxSocketError(QAbstractSocket::SocketError error);
        void rxReadData();

        #ifdef USE_CONTROL_SOCKET
        void ctSocketConnected();
        void ctSocketDisconnected();
        void ctSocketError(QAbstractSocket::SocketError error);
        void ctReadData();
        #endif

        void serialDataAvailable();
        void checkForConnected();
        #ifdef USE_CONTROL_SOCKET
        void sendPing();
        #endif

    public slots:
        void doConnect(QString &hostName, QString &serialPortName);
        void doDisconnect();

    signals:
        void connectError(QString error);
};

#endif // MYTHREAD_H
