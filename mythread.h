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
        bool tcpConnect(const QString &host, const int tcpTxPort, const int tcpRxPort);
        void tcpDisconnect();
        bool serialConnect(const QString &virtualSerialPortName,
                            const QSerialPort::BaudRate baudRate,
                            const QSerialPort::DataBits dataBits,
                            const QSerialPort::StopBits stopBits,
                            const QSerialPort::Parity parity,
                            const QSerialPort::FlowControl flowControl);
        void serialDisconnect();

    private:
        QTcpSocket *m_txSocket = nullptr;
        QTcpSocket *m_rxSocket = nullptr;
        QSerialPort *m_virtualSerialPort = nullptr;
        QString m_host = "";
        int m_tcpTxPort = 0;
        int m_tcpRxPort = 0;
        QString m_virtualSerialPortName = "";
        QSerialPort::BaudRate m_baudRate = QSerialPort::Baud115200;
        QSerialPort::DataBits m_dataBits = QSerialPort::Data8;
        QSerialPort::StopBits m_stopBits = QSerialPort::OneStop;
        QSerialPort::Parity m_parity = QSerialPort::NoParity;
        QSerialPort::FlowControl m_flowControl = QSerialPort::NoFlowControl;

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
};

#endif // MYTHREAD_H
