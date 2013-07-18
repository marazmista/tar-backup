#ifndef TAR_BACKUP_H
#define TAR_BACKUP_H

#include <QMainWindow>
#include <QString>
#include <QStringList>

namespace Ui {
class tar_backup;
}

class tar_backup : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit tar_backup(QWidget *parent = 0);
    ~tar_backup();

    QString getSpeed();
private slots:
    void tarUpdateOutput();
    void tarComplete();
    void encUpdateOutput();
    void encComplete();
    void tarRestoreUpdateOutput();
    void tarRestoreComplete();
    void decryptComplete();
    void decryptUpdateOutput();
    void tarListProcUpdateOutput();
    void displayProgress();

    void on_btn_addProfile_clicked();

    void on_btn_removeProfile_clicked();

    void on_btn_modifyProfile_clicked();

    void on_btn_run_clicked();

    void on_btn_abort_clicked();

    void on_btn_selectFileRestore_clicked();

    void on_btn_setDestRestore_clicked();

    void on_btn_runRestore_clicked();

    void on_btn_listMembers_clicked();

    void on_btn_saveMembersToFile_clicked();

    void on_btn_saveOutput_clicked();

private:
    Ui::tar_backup *ui;
    // var from config //
    QString c_method,e_method,dest,profileName, tarExtraParam, excludeParams;
    bool compress,encrypt, excludeCaches, oneFilesystem, showTotals, excludeVcs, excludeBackups, preservePermissions;

    QString pass, fullFileName,decryptedFullFileName;
    bool decryptOk, canEncrypt;
    qint64 tarArchiveSize,fiSizeNow,fiSizeOld;
    static const ushort tInterval = 1000;

    void loadUiIcons();
    void readBackupProfiles();
    void saveBackupProfiles();
    void readProfileSettings();
    void runDecrypt(const QString &file);
    QString figureOutFileName();
    QString setStatus(QString,bool);
    void setupProcSignals();
    bool checkForRunningJobs();
    QString resolveOptionsParams();
    QString getSpeed(const qint64 &fi1, const qint64 &fi2);
    QStringList stringListFromFile(const QString &fPath);
    void removeIncompleteFiles(bool, bool);
};

#endif // TAR_BACKUP_H
