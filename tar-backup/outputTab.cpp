
// copy marazmista @ 10.07.2013

#include "tar_backup.h"
#include "ui_tar_backup.h"

#include <QProcess>
#include <QFileInfo>
#include <QDate>
#include <QTimer>
#include <QMessageBox>

extern QProcess *tarProc;
extern QProcess *encryptProc;
extern QProcess *tarRestoreProc;
extern QProcess *decryptProc;

extern QTimer *timer;

void tar_backup::on_btn_abort_clicked()
{
    if (!checkForRunnungJobs()) {
        QMessageBox::information(this,"Info","No jobs to abort",QMessageBox::Ok);
        return;
    }

    tarProc->kill();
    tarRestoreProc->kill();
    encryptProc->kill();
    timer->stop();
    ui->label_status->setText(setStatus("Canceled by user.",true));
    ui->label_process->clear();

}

void tar_backup::displayTarSize()
{
    QFileInfo fi(this->dest + fullFileName);
    fiSizeNow = fi.size() / 1000;

    QString speed;
    if ((fiSizeNow - fiSizeOld) > 2000)
        speed = QString::number((fiSizeNow - fiSizeOld)/1000) + " MB/s";
    else
        speed = QString::number(fiSizeNow - fiSizeOld) + " kB/s";

    ui->label_process->setText("Archive size: " + QString::number(fiSizeNow / 1000) + " MB ( " + speed + " )");
    fiSizeOld = fi.size() / 1000;
}

void tar_backup::displayEncTarSize()
{
    QFileInfo fi(this->dest + fullFileName + ".enc_"+ this->e_method);
    ui->label_process->setText("Processed so far: " + QString::number(fi.size() / 1000 / 1000) +
                               " MB / " + QString::number(tarArchiveSize / 1000 / 1000) + " MB");
}

QString tar_backup::setStatus(QString status, bool finishStatus)
{
    if (finishStatus)
        return QDateTime().currentDateTime().toString("hh:mm:ss") + "  |  " + status;
    else
        return "Started at: " + QDateTime().currentDateTime().toString("hh:mm:ss") + " | " + status;
}
