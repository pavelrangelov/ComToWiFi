#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
    class SettingsDialog;
}

//-----------------------------------------------------------------------------
class SettingsDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit SettingsDialog(QWidget *parent = nullptr);
        ~SettingsDialog();
        void setSerialPortName(QString &serialPortName);
        QString serialPortName();

    private:
        Ui::SettingsDialog *ui;
        QStringList getPortNames();
};

#endif // SETTINGSDIALOG_H
