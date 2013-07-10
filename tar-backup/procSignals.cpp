
// copy marazmista @ 10.07.2013

#include "tar_backup.h"
#include "ui_tar_backup.h"

#include <QProcess>
#include <QTimer>
#include <QFileInfo>

extern QProcess *tarProc;
extern QProcess *encryptProc;
extern QProcess *tarRestoreProc;
extern QProcess *decryptProc;

extern QTimer *timer;

void tar_backup::tarRestoreComplete()
{
    if (tarRestoreProc->exitStatus() == 0) {
        ui->label_status->setText("Done.");
        enableButtons(true);
    }
}

void tar_backup::tarComplete()
{
    if (tarProc->exitStatus() == 0) {
        if (canEncrypt) {
            timer->stop();
            tarArchiveSize = QFileInfo(this->dest + fullFileName).size();

            ui->label_status->setText(setStatus("Encrypting...",false));
            connect(encryptProc,SIGNAL(readyReadStandardOutput()),this, SLOT(encUpdateOutput()));
            connect(encryptProc,SIGNAL(finished(int,QProcess::ExitStatus)),this, SLOT(encComplete()));
            connect(timer,SIGNAL(timeout()),this,SLOT(displayEncTarSize()));

            QString encryptCmd = "openssl "+ e_method + " -salt -pass pass:" + pass +
                    " -in \"" + this->dest + fullFileName + "\"" +
                    " -out \"" + this->dest + fullFileName + ".enc_"+ this->e_method;

            timer->start();
            encryptProc->start(encryptCmd,QProcess::ReadWrite);
        } else {
            ui->label_status->setText(setStatus("Done.",true));
            enableButtons(true);
        }
    }
}

void tar_backup::encComplete()
{
    if (encryptProc->exitStatus() == 0) {
        QFile tarDel(this->dest + fullFileName);
        tarDel.remove();
        tarDel.close();

        ui->label_status->setText(setStatus("Encryption done.",true));
        enableButtons(true);
        timer->stop();
        ui->label_process->clear();
    }
}

void tar_backup::decryptComplete()
{
    if (decryptProc->exitStatus() == 0) {
        if (decryptOk) {
            ui->label_status->setText(setStatus("Decrypt done.",true));
            ui->t_restoreFile->setText(decryptedFullFileName);
            if (!ui->cb_decryptOnly->isChecked())
                on_btn_runRestore_clicked();
        }
    }
}

void tar_backup::tarRestoreUpdateOutput()
{
    QString s = tarRestoreProc->readAllStandardOutput();
    if (!s.isEmpty())
        ui->outputT->appendPlainText(s);
}


void tar_backup::encUpdateOutput()
{
    QString s = encryptProc->readAllStandardOutput();
    if (!s.isEmpty())
        ui->outputT->appendPlainText(s);
}

void tar_backup::decryptUpdateOutput()
{
    QString s = decryptProc->readAllStandardOutput();
    if (!s.isEmpty()) {
        ui->outputT->appendPlainText(s);
        if (s.contains("bad decrypt")) {
            ui->label_status->setText(setStatus("Decrypt filed. (bad password?)",true));
            decryptOk = false;
        }
    }
}

void tar_backup::tarUpdateOutput()
{
    QString s = tarProc->readAllStandardOutput();
    if (!s.isEmpty())
        ui->outputT->appendPlainText(s);
}
