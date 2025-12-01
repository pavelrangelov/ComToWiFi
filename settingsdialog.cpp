#include <QSerialPort>
#include <QSerialPortInfo>
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

//-----------------------------------------------------------------------------
SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SettingsDialog) {
    ui->setupUi(this);
    setWindowTitle(tr("Serial Port Settings"));
    ui->comboSerialPort->addItems(getPortNames());
    ui->comboSerialPort->setEditable(true);
}

//-----------------------------------------------------------------------------
SettingsDialog::~SettingsDialog() {
    delete ui;
}

//-----------------------------------------------------------------------------
QStringList SettingsDialog::getPortNames() {
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
void SettingsDialog::setSerialPortName(QString &serialPortName) {
    ui->comboSerialPort->setCurrentText(serialPortName);
}

//-----------------------------------------------------------------------------
QString SettingsDialog::serialPortName() {
    return ui->comboSerialPort->currentText();
}
