
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
    if (!checkForRunningJobs()) {
        QMessageBox::information(this,"Info","No jobs to abort",QMessageBox::Ok);
        return;
    }

    if (tarProc->state() == QProcess::Running)
        tarProc->kill();
    if (tarRestoreProc->state() == QProcess::Running)
        tarRestoreProc->kill();
    if (encryptProc->state() == QProcess::Running)
        encryptProc->kill();
    if (decryptProc->state() == QProcess::Running)
        decryptProc->kill();

    timer->stop();
    ui->label_status->setText(setStatus("Canceled by user.",true));
    ui->label_process->clear();
}

QString tar_backup::getSpeed(const qint64 &fi1, const qint64 &fi2)
{
    QString speed;
    if ((fi1 - fi2) > 2000)
        speed = QString::number((fi1 - fi2)/1000) + " MB/s";
    else
        speed = QString::number(fi1 - fi2) + " kB/s";

    return speed;
}

void tar_backup::displayProgress()
{
    if (QProcess::Running == tarProc->state()) {
        QFileInfo fi(this->dest + fullFileName);
        fiSizeNow = fi.size() / 1000;

        ui->label_process->setText("Archive size: " + QString::number(fiSizeNow / 1000) + " MB ( " + getSpeed(fiSizeNow,fiSizeOld) + " )");
        fiSizeOld = fi.size() / 1000;
        return;
    }

    if (QProcess::Running == encryptProc->state()) {
        QFileInfo fi(this->dest + fullFileName + ".enc_"+ this->e_method);
        fiSizeNow = fi.size() / 1000;
        ui->label_process->setText("Processed so far: " + QString::number(fi.size() / 1000 / 1000) +
                                   " MB / " + QString::number(tarArchiveSize / 1000 / 1000) + " MB ( " + getSpeed(fiSizeNow,fiSizeOld) + " )");
        fiSizeOld = fi.size() / 1000;
        return;
    }
}

QString tar_backup::setStatus(QString status, bool finishStatus)
{
    if (finishStatus)
        return QDateTime().currentDateTime().toString("hh:mm:ss") + "  |  " + status;
    else
        return "Started at: " + QDateTime().currentDateTime().toString("hh:mm:ss") + " | " + status;
}
