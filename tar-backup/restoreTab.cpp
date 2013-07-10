
// copy marazmista @ 10.07.2013

#include "tar_backup.h"
#include "ui_tar_backup.h"

#include <QProcess>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>

QProcess *decryptProc = new QProcess();
QProcess *tarRestoreProc = new QProcess();

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
        connect(decryptProc,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(decryptComplete()));
        connect(decryptProc,SIGNAL(readyReadStandardOutput()),this,SLOT(decryptUpdateOutput()));

        decryptProc->start(decryptCmd,QProcess::ReadWrite);
    } else
        return;
}

void tar_backup::on_btn_runRestore_clicked()
{
    QString fName = ui->t_restoreFile->text();
    QString dest = ui->t_destRestore->text();

    // check valid data to restore //
    if (fName.isEmpty()) {
        QMessageBox::critical(this,"Error", "Source archive not set!",QMessageBox::Ok);
        return;
    }
    if (dest.isEmpty()) {
        QMessageBox::critical(this,"Error", "Destination archive not set!",QMessageBox::Ok);
        return;
    }

    decryptOk = true;

    ui->outputT->clear();
    ui->label_status->setText(setStatus("Restoring...",false));
    enableButtons(false);
    ui->tabWidget->setCurrentIndex(2);

    tarRestoreProc->setReadChannelMode(QProcess::MergedChannels);
    connect(tarRestoreProc,SIGNAL(readyReadStandardOutput()),this,SLOT(tarRestoreUpdateOutput()));
    connect(tarRestoreProc,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(tarRestoreComplete()));

    if (fName.contains(".enc_")) {
        runDecrypt(fName);
    } else {
        QString restoreCmd = "tar -xvpf " + fName + " -C "+ dest;
        tarRestoreProc->start(restoreCmd,QProcess::ReadWrite);
    }
}
