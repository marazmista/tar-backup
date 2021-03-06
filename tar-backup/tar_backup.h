#ifndef TAR_BACKUP_H
#define TAR_BACKUP_H

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QListWidgetItem>

namespace Ui {
class tar_backup;
}

class tar_backup : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit tar_backup(QWidget *parent = 0);
    ~tar_backup();

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
    void on_cb_decryptOnly_clicked();
    void on_btn_showActivityLog_clicked();
    void on_btn_abortListMembers_clicked();

private:
    Ui::tar_backup *ui;
    // var from config //
    QString c_method,e_method,dest,profileName, tarExtraParam, excludeParams;
    bool compress,encrypt, excludeCaches, oneFilesystem, showTotals, excludeVcs, excludeBackups, preservePermissions, passFromFile;

    QString pass, fullFileName,decryptedFullFileName;
    bool decryptOk, canEncrypt;
    qint64 tarArchiveSize,fiSizeNow,fiSizeOld, encryptedArchiveSize;
    static const ushort tInterval = 1000;

    void readBackupProfiles();
    void saveBackupProfiles();
    void readProfileSettings(const QString);
    void setLastBackupDate(QTreeWidgetItem *);
    void runDecrypt(const QString &file);
    QString figureOutFileName() const;
    QString setStatus(const QString, const bool) const;
    void setupProcSignals();
    bool checkForRunningJobs() const;
    QString resolveOptionsParams();
    QString getSpeed(const qint64 &fi1, const qint64 &fi2) const;
    QString getDate(const bool) const;
    QStringList stringListFromFile(const QString &fPath);
    void removeIncompleteFiles(const bool, const bool) const;
    void beginEncryptArchive();
    QString getSpeed();
    QString askForPassword(const bool);
};

#endif // TAR_BACKUP_H
