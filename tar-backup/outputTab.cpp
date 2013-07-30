
// copy marazmista @ 10.07.2013

#include "tar_backup.h"
#include "ui_tar_backup.h"

#include <QProcess>
#include <QFileInfo>
#include <QDate>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>

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

    if (QMessageBox::No == QMessageBox::question(this,"Question","Abort current job and delete incomplete archive?",QMessageBox::Yes,QMessageBox::No))
        return;

    if (tarProc->state() == QProcess::Running) {
        tarProc->kill();
        removeIncompleteFiles(true, false);
    }
    if (encryptProc->state() == QProcess::Running) {
        encryptProc->kill();
        removeIncompleteFiles(true, true);
    }
    if (tarRestoreProc->state() == QProcess::Running)
        tarRestoreProc->kill();
    if (decryptProc->state() == QProcess::Running)
        decryptProc->kill();

    timer->stop();
    ui->label_status->setText(setStatus("Canceled by user.",true));
    ui->label_process->clear();
}

void tar_backup::removeIncompleteFiles(const bool fArch, const bool fEncArch) const {
    if (fArch) {
        QFile f(this->dest + this->fullFileName);
        if (f.exists())
            f.remove();
        f.close();
    }
    if (fEncArch) {
        QFile f(this->dest + this->fullFileName +".enc_" + this->e_method);
        if (f.exists())
            f.remove();
        f.close();
    }
}

QString tar_backup::getSpeed(const qint64 &fi1, const qint64 &fi2) const
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

QString tar_backup::setStatus(const QString status, const bool finishStatus) const  {
    QString s;

    if (finishStatus) {
        s = QDateTime().currentDateTime().toString("hh:mm:ss") + "  |  " + status;
        ui->list_activityLog->addItem(s);
        return s;
    } else {
        s = "Started at: " + QDateTime().currentDateTime().toString("hh:mm:ss") + " | " + status;
        ui->list_activityLog->addItem(s);
        return s;
    }
}

void tar_backup::on_btn_saveOutput_clicked()
{
    QFile f(QFileDialog::getSaveFileName(this,"Save file...","",".txt"));
    f.open(QIODevice::WriteOnly);
    f.write(ui->outputT->toPlainText().toLatin1());
    f.close();
}
