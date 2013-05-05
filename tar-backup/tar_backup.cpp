
// copy marazmista @ 04.2013

#include "tar_backup.h"
#include "ui_tar_backup.h"
#include "adddialog.h"

#include <QFile>
#include <QSettings>
#include <QTextStream>
#include <QProcess>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>

QProcess *tarProc = new QProcess();
QProcess *encryptProc = new QProcess();
QProcess *decryptProc = new QProcess();
QProcess *tarRestoreProc = new QProcess();
QString pass, fullFileName,decryptedFullFileName;
bool canEncrypt,decryptOk;

tar_backup::tar_backup(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::tar_backup)
{
    ui->setupUi(this);
    readBackupProfiles();
}

tar_backup::~tar_backup()
{
    delete ui;
}

//*****************
// manage profiles //

void tar_backup::readProfileSettings()
{
    this->profileName = ui->list_backupProfiles->currentItem()->text();

    QSettings pSet(QApplication::applicationDirPath() + "/"+ this->profileName+".ini",QSettings::IniFormat);
    this->dest = pSet.value("destination").toString();
    this->compress = pSet.value("compression", true).toBool();
    this->encrypt = pSet.value("encryption",false).toBool();
    this->c_method = pSet.value("compression_method","xz").toString();
    this->e_method = pSet.value("encryption_method","aes-256-cbc").toString();
}

void tar_backup::readBackupProfiles()
{
    QFile bpf(QApplication::applicationDirPath() + "/backupProfiles");
    bpf.open(QIODevice::ReadOnly);
    QStringList ls;
    QTextStream ts(&bpf);

    while (true) {
        QString s = ts.readLine();
        if (s.isEmpty())
            break;
        ls.append(s);
    }
    bpf.close();
    ui->list_backupProfiles->addItems(ls);
}

void tar_backup::saveBackupProfiles()
{
    QFile bpf(QApplication::applicationDirPath() + "/backupProfiles");
    bpf.open(QIODevice::WriteOnly);

    QByteArray tmpArr;
    for (int i=0;i<ui->list_backupProfiles->count(); i++) {
        tmpArr.append(QString(ui->list_backupProfiles->item(i)->text()));
        if (i != ui->list_backupProfiles->count() - 1)  // to avoid empty line at end of file
            tmpArr.append('\n');
    }
    bpf.write(tmpArr);
    bpf.close();
}

void tar_backup::on_btn_addProfile_clicked()
{
    addDialog a;
    a.exec();

    ui->list_backupProfiles->addItem(a.backupProfileName);
    saveBackupProfiles();
}

void tar_backup::on_btn_removeProfile_clicked()
{
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

void tar_backup::on_btn_modifyProfile_clicked()
{
    readProfileSettings();
    addDialog a(profileName,dest,compress,c_method,encrypt,e_method);
    a.exec();
}

// manage profiles end //

void tar_backup::on_btn_run_clicked()
{
    readProfileSettings();

    if (!QDir(this->dest).exists()) {
        QMessageBox mb;
        mb.setText("Destination directoryn not exists!");
        mb.setIcon(QMessageBox::Critical);
        mb.show();
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
    connect(tarProc,SIGNAL(readyReadStandardOutput()),this,SLOT(tarUpdateOutput()));
    connect(tarProc,SIGNAL(finished(int,QProcess::ExitStatus)), this,SLOT(tarComplete()));

    ui->label_status->setText("Working...");
    ui->outputT->clear();

    readProfileSettings();
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

    if (compress) {
        if (c_method == "xz")
            tarCmd = "tar -Jcpvf \"" + this->dest + fullFileName + "\" " + targets;
        if (c_method == "bzip2")
            tarCmd = "tar -jcpvf \"" + this->dest + fullFileName + "\" " + targets;
        if (c_method == "gzip")
            tarCmd = "tar -zcpvf \"" + this->dest + fullFileName + "\" " + targets;
    } else
        tarCmd = "tar -cpvf \"" + this->dest + fullFileName + "\" " + targets;

    ui->tabWidget->setCurrentIndex(2);
    enableButtons(false);
    tarProc->start(tarCmd,QProcess::ReadWrite);
}

QString tar_backup::figureOutFileName()
{
    if (!this->compress)
        return this->profileName + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy-hh_mm_ss") + ".tar";
    else {
        if (this->c_method == "xz")
            return this->profileName + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy-hh_mm_ss") + ".tar.xz";
        if (this->c_method == "bzip2")
            return this->profileName + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy-hh_mm_ss") + ".tar.bz2";
        if (this->c_method == "gzip")
            return this->profileName + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy-hh_mm_ss") + ".tar.gz";
    }
}

void tar_backup::on_btn_abort_clicked()
{
    tarProc->kill();
    tarRestoreProc->kill();
    ui->label_status->setText("Canceled by user.");
    enableButtons(true);
}

//*****************
// restore tab //

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

        ui->label_status->setText("Decrypting...");

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
    decryptOk = true;

    if (!fName.isEmpty() && !dest.isEmpty()) {
        ui->outputT->clear();
        ui->label_status->setText("Restoring...");
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
    } else {
        QMessageBox mb;
        mb.setText("File or restore destination not set!");
        mb.setIcon(QMessageBox::Critical);
        mb.exec();
    }

}
// restore tab end //


//*****************
// signals for qprcesses //

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
            ui->label_status->setText("Encrypting...");
            connect(encryptProc,SIGNAL(readyReadStandardOutput()),this, SLOT(encUpdateOutput()));
            connect(encryptProc,SIGNAL(finished(int,QProcess::ExitStatus)),this, SLOT(encComplete()));

            QString encryptCmd = "openssl "+ e_method + " -salt -pass pass:" + pass +
                    " -in \"" + this->dest + fullFileName + "\"" +
                    " -out \"" + this->dest + fullFileName + ".enc_"+ this->e_method;

            encryptProc->start(encryptCmd,QProcess::ReadWrite);
        } else {
            ui->label_status->setText("Done.");
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

        ui->label_status->setText("Done.");
        enableButtons(true);
    }
}

void tar_backup::decryptComplete()
{
    if (decryptProc->exitStatus() == 0) {
        if (decryptOk) {
            ui->label_status->setText("Decrypt done.");
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
            ui->label_status->setText("Decrypt filed. (bad password?)");
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

// signals for qprcesses end //

void tar_backup::enableButtons(bool val)
{
    ui->btn_run->setEnabled(val);
    ui->btn_runRestore->setEnabled(val);
}
