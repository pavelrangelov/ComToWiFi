#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QSerialPort>

class MyThread : public QThread {
    Q_OBJECT

    public:
        MyThread(QObject *parent = nullptr);
        ~MyThread();
        bool tcpConnect();
        void tcpDisconnect();
        bool serialConnect();
        void serialDisconnect();

    private:
        QTcpSocket *m_txSocket = nullptr;
        QTcpSocket *m_rxSocket = nullptr;
        QSerialPort *m_virtualSerialPort = nullptr;
        QString m_host = "";
        int m_espTxPort = 0;
        int m_espRxPort = 0;
        QString m_virtualSerialPortName = "";

        QString createSerialPortName(QString name);

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

        void serialDataAvailable();

    public slots:
        void doConnect(QString &hostName, QString &serialPortName);
        void doDisconnect();

    signals:
        void connectError(QString error);
};

#endif // MYTHREAD_H
