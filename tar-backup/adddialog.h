#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class addDialog;
}

class addDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit addDialog(QWidget *parent = 0);
    explicit addDialog(QString &profileName,QString &dest,bool &compression, QString &c_method,
                       bool &encryption, QString &e_method, QString &tarExtraParam, bool &excludeCaches,
                       bool &oneFilesystem, bool &showTotals, QWidget *parent = 0);
    ~addDialog();
    QString backupProfileName;
    
    void setupSignals();
private slots:
    void deleteKeyOnList();
    void on_btn_save_clicked();

    void on_btn_addFolders_clicked();

    void on_btn_addFiles_clicked();

    void on_btn_cancel_clicked();

    void on_btn_remove_clicked();

    void on_btn_setDest_clicked();

    void on_btn_tarManpage_clicked();

private:
    Ui::addDialog *ui;
    void loadUiIcons();
};

#endif // ADDDIALOG_H
