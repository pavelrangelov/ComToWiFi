#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QSerialPort>
#include <QTimer>

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
        QSerialPort *m_virtualSerialPort = nullptr;
        QString m_host = "";
        QString m_virtualSerialPortName = "";
        QTimer *m_connectTimer;

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
        void checkForConnected();

    public slots:
        void doConnect(QString &hostName, QString &serialPortName);
        void doDisconnect();

    signals:
        void connectError(QString error);
};

#endif // MYTHREAD_H
