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
        explicit SettingsDialog(QString serialPortName = "", QWidget *parent = nullptr);
        ~SettingsDialog();
        QString serialPortName();

    private:
        Ui::SettingsDialog *ui;
        QStringList getPortNames();
};

#endif // SETTINGSDIALOG_H
