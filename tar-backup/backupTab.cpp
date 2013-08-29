
// copy marazmista @ 10.07.2013

#include "tar_backup.h"
#include "ui_tar_backup.h"
#include "adddialog.h"

#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QTimer>
#include <QDate>

QProcess *tarProc = new QProcess();
QProcess *encryptProc = new QProcess();

QTimer *timer = new QTimer();

void tar_backup::on_btn_addProfile_clicked()
{
    addDialog a;
    a.exec();

    if (a.result() == 1) {
        ui->list_backupProfiles->addItem(a.backupProfileName + " (Last backup: never)");
        saveBackupProfiles();
    }  
}

void tar_backup::on_btn_removeProfile_clicked()
{
    if (!ui->list_backupProfiles->currentItem() != 0) //check if something is selected
        return;

    if (QMessageBox::Yes == QMessageBox::question(this,
                              "Queston",
                              "Remove profile " + figureOutProfileName(ui->list_backupProfiles->currentItem()) + "?",
                              QMessageBox::Yes,QMessageBox::No)) {
        QFile f(QDir::homePath() + "/.tar-backup/" +
                figureOutProfileName(ui->list_backupProfiles->currentItem()));
        f.remove();
        f.close();

        QFile fPat(QDir::homePath() + "/.tar-backup/" + figureOutProfileName(ui->list_backupProfiles->currentItem()) + "-excludePatterns");
        fPat.remove();
        fPat.close();

        QFile fini(QDir::homePath() + "/.tar-backup/" +
                   figureOutProfileName(ui->list_backupProfiles->currentItem())+".ini");
        fini.remove();
        fini.close();

        delete ui->list_backupProfiles->currentItem();
        saveBackupProfiles();
    }
}

void tar_backup::on_btn_modifyProfile_clicked()
{
    if (!ui->list_backupProfiles->currentItem() != 0) //check if something is selected
        return;

    readProfileSettings(figureOutProfileName(ui->list_backupProfiles->currentItem()));
    addDialog a(profileName,dest,compress,c_method,encrypt,e_method,tarExtraParam,
                excludeCaches,excludeVcs,excludeBackups,oneFilesystem,showTotals,preservePermissions, passFromFile);
    a.exec();

    if (a.result() == 1) {
        if (figureOutProfileName(ui->list_backupProfiles->currentItem()) != a.backupProfileName) {
            ui->list_backupProfiles->addItem(a.backupProfileName + " (Last backup: never)");
            saveBackupProfiles();
        }
    }
}

// manage profiles end //

void tar_backup::on_btn_run_clicked()
{
    if (!ui->list_backupProfiles->currentItem() != 0) //check if something is selected
        return;

    if (checkForRunningJobs()) {
        QMessageBox::critical(this, "Error", "Other job is currently running",QMessageBox::Ok);
        return;
    }

    canEncrypt = false;
    readProfileSettings(figureOutProfileName(ui->list_backupProfiles->currentItem()));
    tarArchiveSize = 0;
    fiSizeNow = fiSizeOld = 0;

    if (!QDir(this->dest).exists()) {
        QMessageBox::critical(this,"Error", "Destination directory not exists!",QMessageBox::Ok);
        return;
    }

    if (encrypt) {
        if (!askForPassword(this->passFromFile).isNull())
            canEncrypt = true;
        else
            return;
    }

    tarProc->setProcessChannelMode(QProcess::MergedChannels);

    ui->label_status->setText(setStatus("Creating backup (" + figureOutProfileName(ui->list_backupProfiles->currentItem()) + ")...",false));
    ui->outputT->clear();
    ui->label_process->clear();

    this->fullFileName = figureOutFileName();

    QString tarCmd, targets = QDir::homePath() + "/.tar-backup/" + this->profileName;

    timer->setInterval(tInterval);

    if (compress)
        tarCmd = "tar --create -v --" + this->c_method + " " + resolveOptionsParams() + this->excludeParams + this->tarExtraParam +" --file \"" + this->dest + this->fullFileName + "\" -T \"" + targets;
    else
        tarCmd = "tar --create -v " + resolveOptionsParams() + this->excludeParams + this->tarExtraParam +"--file \"" + this->dest + this->fullFileName + "\" -T \"" + targets;

    setLastBackupDate(ui->list_backupProfiles->currentItem());
    ui->tabWidget->setCurrentIndex(2);
    timer->start();
    tarProc->start(tarCmd,QProcess::ReadWrite);
}

void tar_backup::setLastBackupDate(QListWidgetItem *currentItem) {
    currentItem->setText(figureOutProfileName(currentItem) + " (Last backup: "+ getDate(false));
}

QString tar_backup::figureOutFileName() const {
    if (!this->compress)
        return this->profileName + "_" + getDate(true) + ".tar";
    else {
        if (this->c_method == "xz")
            return this->profileName + "_" + getDate(true) + ".tar.xz";
        if (this->c_method == "lzma")
            return this->profileName + "_" + getDate(true) + ".tar.lzma";
        if (this->c_method == "bzip2")
            return this->profileName + "_" + getDate(true) + ".tar.bz2";
        if (this->c_method == "gzip")
            return this->profileName + "_" + getDate(true) + ".tar.gz";
    }
    return QString::null;  //avoid compile warrning
}

QString tar_backup::getDate(const bool forFileName) const {
    if (forFileName)
        return QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss"); // used in FigureOutFileName()
    else
        return QDateTime::currentDateTime().toString("dd.MM.yyyy - hh:mm:ss") + ")"; // used for save last backup date
}

QString tar_backup::figureOutProfileName(const QListWidgetItem *selectedProfile) const {
    return selectedProfile->text().split(" (Last backup: ",QString::SkipEmptyParts,Qt::CaseInsensitive)[0];
}

QString tar_backup::resolveOptionsParams() {
    QString params;

    if (this->excludeCaches)
        params += "--exclude-caches-all ";
    if (this->excludeVcs)
        params += "--exclude-vcs ";
    if (this->excludeBackups)
        params += "--exclude-backups ";
    if (this->oneFilesystem)
        params += "--one-file-system ";
    if (this->showTotals)
        params += "--totals ";
    if (this->preservePermissions)
        params += "--preserve-permissions ";

    return params;

}

void tar_backup::beginEncryptArchive()
{
    timer->stop();
    tarArchiveSize = QFileInfo(this->dest + this->fullFileName).size();
    fiSizeNow = fiSizeOld = 0;

    ui->label_status->setText(setStatus("Encrypting archive (" + QString::number(tarArchiveSize / 1000 / 1000) + "MB)...",false));
    QString encryptCmd = "openssl "+ this->e_method + " -salt -pass " + this->pass +
            " -in \"" + this->dest + this->fullFileName + "\"" +
            " -out \"" + this->dest + this->fullFileName + ".enc_"+ this->e_method;

    timer->start();
    encryptProc->start(encryptCmd,QProcess::ReadWrite);
}
