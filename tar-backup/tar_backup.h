#ifndef TAR_BACKUP_H
#define TAR_BACKUP_H

#include <QMainWindow>
#include <QString>

namespace Ui {
class tar_backup;
}

class tar_backup : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit tar_backup(QWidget *parent = 0);
    ~tar_backup();
    void readBackupProfiles();
    void saveBackupProfiles();
    void readProfileSettings();
    void runDecrypt(const QString &file);
    QString figureOutFileName();
    QString setStatus(QString,bool);

    void enableButtons(bool);
private slots:
    void tarUpdateOutput();
    void tarComplete();
    void encUpdateOutput();
    void encComplete();
    void tarRestoreUpdateOutput();
    void tarRestoreComplete();
    void decryptComplete();
    void decryptUpdateOutput();
    void displayTarSize();
    void displayEncTarSize();

    void on_btn_addProfile_clicked();

    void on_btn_removeProfile_clicked();

    void on_btn_modifyProfile_clicked();

    void on_btn_run_clicked();

    void on_btn_abort_clicked();

    void on_btn_selectFileRestore_clicked();

    void on_btn_setDestRestore_clicked();

    void on_btn_runRestore_clicked();

private:
    Ui::tar_backup *ui;
    QString c_method,e_method,dest,profileName;
    bool compress,encrypt;
};

#endif // TAR_BACKUP_H
