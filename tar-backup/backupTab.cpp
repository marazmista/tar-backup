
// copy marazmista @ 10.07.2013

#include "tar_backup.h"
#include "ui_tar_backup.h"
#include "adddialog.h"

#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QInputDialog>
#include <QTimer>
#include <QTextStream>
#include <QDate>

QProcess *tarProc = new QProcess();
QProcess *encryptProc = new QProcess();

QTimer *timer = new QTimer();

void tar_backup::on_btn_addProfile_clicked()
{
    addDialog a;
    a.exec();

    if (a.result() == 1) {
        ui->list_backupProfiles->addItem(a.backupProfileName);
        saveBackupProfiles();
    }
}

void tar_backup::on_btn_removeProfile_clicked()
{
    if (!ui->list_backupProfiles->currentItem() != 0) //check if something is selected
        return;

    if (QMessageBox::Yes == QMessageBox::question(this,
                              "Queston",
                              "Remove profile " + ui->list_backupProfiles->currentItem()->text() + "?",
                              QMessageBox::Yes,QMessageBox::No)) {
        QFile f(QApplication::applicationDirPath() + "/" +
                ui->list_backupProfiles->currentItem()->text());
        f.remove();
        f.close();
        QFile fini(QApplication::applicationDirPath() + "/" +
                   ui->list_backupProfiles->currentItem()->text()+".ini");
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

    readProfileSettings();
    addDialog a(profileName,dest,compress,c_method,encrypt,e_method,tarExtraParam);
    a.exec();
}

// manage profiles end //

void tar_backup::on_btn_run_clicked()
{
    if (!ui->list_backupProfiles->currentItem() != 0) //check if something is selected
        return;

    readProfileSettings();
    tarArchiveSize = 0;
    fiSizeNow = fiSizeOld = 0;

    if (!QDir(this->dest).exists()) {
        QMessageBox::critical(this,"Error", "Destination directory not exists!",QMessageBox::Ok);
        return;
    }

    if (encrypt) {
        bool ok;
        pass = QInputDialog::getText(this,"Password for encryption",
                                     "Password",
                                     QLineEdit::Normal,
                                     "",
                                     &ok);
        if (ok && !pass.isEmpty()) {
            canEncrypt = true;
        } else
            return;
    }

    tarProc->setProcessChannelMode(QProcess::MergedChannels);

    ui->label_status->setText(setStatus("Working...",false));
    ui->outputT->clear();
    ui->label_process->clear();

    fullFileName = figureOutFileName();

    QFile profileFolderBackupList(QApplication::applicationDirPath() + "/" + this->profileName);
    profileFolderBackupList.open(QIODevice::ReadOnly);
    QTextStream ts(&profileFolderBackupList);

    QString tarCmd, targets;
    while (true) {
        QString s = ts.readLine();
        if (s.isEmpty())
            break;
        targets.append(" \""+s+"\"");
    }
    profileFolderBackupList.close();

    timer->setInterval(tInterval);

    if (compress)
        tarCmd = "tar --create --" + c_method + " -p -v "+ this->tarExtraParam +" --file \"" + this->dest + fullFileName + "\" " + targets;
    else
        tarCmd = "tar --create -p -v --file \"" + this->dest + fullFileName + "\" " + targets;

    ui->tabWidget->setCurrentIndex(2);
    enableButtons(false);
    timer->start();
    tarProc->start(tarCmd,QProcess::ReadWrite);
}

QString tar_backup::figureOutFileName()
{
    if (!this->compress)
        return this->profileName + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy-hh_mm_ss") + ".tar";
    else {
        if (this->c_method == "xz")
            return this->profileName + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy-hh_mm_ss") + ".tar.xz";
        if (this->c_method == "lzma")
            return this->profileName + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy-hh_mm_ss") + ".tar.lzma";
        if (this->c_method == "bzip2")
            return this->profileName + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy-hh_mm_ss") + ".tar.bz2";
        if (this->c_method == "gzip")
            return this->profileName + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy-hh_mm_ss") + ".tar.gz";
    }
    return QString::null;  //avoid compile warrning
}

