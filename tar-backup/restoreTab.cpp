
// copy marazmista @ 10.07.2013

#include "tar_backup.h"
#include "ui_tar_backup.h"

#include <QProcess>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>

QProcess *decryptProc = new QProcess();
QProcess *tarRestoreProc = new QProcess();
QProcess *tarListProc = new QProcess();

void tar_backup::on_btn_selectFileRestore_clicked()
{
    QFileDialog fd;
    fd.setFileMode(QFileDialog::ExistingFile);
    if (fd.exec())
        ui->t_restoreFile->setText(fd.selectedFiles()[0]);
}

void tar_backup::on_btn_setDestRestore_clicked()
{
    ui->t_destRestore->setText(QFileDialog::getExistingDirectory(this,
                                                                 "Select directory",
                                                                 "",
                                                                 QFileDialog::ShowDirsOnly));
}

void tar_backup::runDecrypt(const QString &file)
{
    QString pass, d_method, decryptCmd;
    bool ok;

    pass = QInputDialog::getText(this,"Enter password","Password:", QLineEdit::Normal,"",&ok);

    if (ok && !pass.isEmpty()) {
        if (file.contains("_aes-256-cbc"))
            d_method = "aes-256-cbc";
        if (file.contains("_rc4"))
            d_method = "rc4";
        if (file.contains("_des"))
            d_method = "des";
        if (file.contains("_des3"))
            d_method = "des3";

        decryptedFullFileName = ui->t_destRestore->text() + "/" + QFileInfo(file).fileName().remove(".enc_"+d_method);

        decryptCmd = "openssl " + d_method + " -d -pass pass:" + pass + " -in \"" + file +
                "\" -out \"" + decryptedFullFileName +"\"";

        ui->label_status->setText(setStatus("Decrypting...",false));

        decryptProc->setReadChannelMode(QProcess::MergedChannels);
        decryptProc->start(decryptCmd,QProcess::ReadWrite);
    } else
        return;
}

void tar_backup::on_btn_runRestore_clicked()
{
    if (checkForRunningJobs()) {
        QMessageBox::critical(this,"Error", "Other job is currently running", QMessageBox::Ok);
        return;
    }

    QString fName = ui->t_restoreFile->text();
    QString dest = ui->t_destRestore->text();

    // check valid data to restore //
    if (fName.isEmpty()) {
        QMessageBox::critical(this,"Error", "Source archive not set!",QMessageBox::Ok);
        return;
    }
    if (QFile::exists(fName) == false) {
        QMessageBox::critical(this,"Error", "Archive file not exists!",QMessageBox::Ok);
        return;
    }
    if (dest.isEmpty()) {
        QMessageBox::critical(this,"Error", "Destination archive not set!",QMessageBox::Ok);
        return;
    }

    decryptOk = true;

    ui->outputT->clear();
    ui->label_status->setText(setStatus("Restoring...",false));
    ui->label_process->setText(QString::null);
    ui->tabWidget->setCurrentIndex(2);

    tarRestoreProc->setReadChannelMode(QProcess::MergedChannels);

    if (fName.contains(".enc_")) {
        runDecrypt(fName);
    } else {
        QString restoreCmd = "tar --extract -v -p --file " + fName + " -C "+ dest;
        tarRestoreProc->start(restoreCmd,QProcess::ReadWrite);
    }
}


// list members tab //

void tar_backup::on_btn_listMembers_clicked()
{
    QString fName = ui->t_restoreFile->text();

    if (fName.isEmpty()) {
        QMessageBox::critical(this,"Error", "Source archive not set!",QMessageBox::Ok);
        return;
    }

    if (fName.contains(".enc_")) {
        QMessageBox::information(this,"Info", "Please, decrypt archive first.", QMessageBox::Ok);
        return;
    }


    QString verbose;
    if (ui->cb_listExtendedInfo->isChecked())
        verbose = " -v ";
    else
        verbose = "";

    QString cmd = "tar --list " + verbose + " --file " + fName + "\"";
    ui->tarMembersT->clear();

    tarListProc->setProcessChannelMode(QProcess::MergedChannels);
    tarListProc->start(cmd,QIODevice::ReadOnly);
}


void tar_backup::on_btn_saveMembersToFile_clicked()
{
    QFile f(QFileDialog::getSaveFileName(this,"Save file...","",".txt"));
    f.open(QIODevice::WriteOnly);
    f.write(ui->tarMembersT->toPlainText().toLatin1());
    f.close();
}

