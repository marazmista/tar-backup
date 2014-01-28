
// copy marazmista @ 04.2013

#include "tar_backup.h"
#include "ui_tar_backup.h"
#include "adddialog.h"

#include <QSettings>
#include <QTextStream>
#include <QFileDialog>
#include <QDir>
#include <QInputDialog>

tar_backup::tar_backup(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::tar_backup)
{
    ui->setupUi(this);
    readBackupProfiles();
    setupProcSignals();
    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget_2->setCurrentIndex(0);

    QDir appHomePath(QDir::homePath() + "/.tar-backup");
    if (!appHomePath.exists())
        appHomePath.mkdir(QDir::homePath() + "/.tar-backup");
}

tar_backup::~tar_backup()
{
    saveBackupProfiles();
    delete ui;
}

//*****************
// manage profiles //

void tar_backup::readProfileSettings(const QString selectedProfileName)
{
    this->profileName = selectedProfileName;

    QSettings pSet(QDir::homePath() + "/.tar-backup/"+ this->profileName+".ini",QSettings::IniFormat);
    this->dest = pSet.value("destination").toString();
    this->compress = pSet.value("compression", true).toBool();
    this->encrypt = pSet.value("encryption",false).toBool();
    this->c_method = pSet.value("compression_method","xz").toString();
    this->e_method = pSet.value("encryption_method","aes-256-cbc").toString();
    this->tarExtraParam = pSet.value("tarExtraParam","").toString();
    this->excludeCaches = pSet.value("excludeCaches",false).toBool();
    this->excludeVcs = pSet.value("excludeVCS", false).toBool();
    this->excludeBackups = pSet.value("excludeBackups", false).toBool();
    this->oneFilesystem = pSet.value("oneFilesystem",false).toBool();
    this->showTotals = pSet.value("showTotals", true).toBool();
    this->preservePermissions = pSet.value("preservePermissions",true).toBool();
    this->passFromFile = pSet.value("passFromFile",false).toBool();

    QStringList excludeList =  stringListFromFile(QDir::homePath() + "/.tar-backup/" + this->profileName + "-excludePatterns");
    if (!excludeList.isEmpty()) {
        excludeParams = "--exclude=";
        excludeParams += excludeList.join(" --exclude=");
    }

}

QStringList tar_backup::stringListFromFile(const QString &fPath)
{
    QFile fList(fPath);
    fList.open(QIODevice::ReadOnly);
    QStringList sl;
    QTextStream ts(&fList);

    while (true)    {
        QString s = ts.readLine();
        if (s.isEmpty())
            break;
        sl.append(s);
    }
    fList.close();

    return sl;
}

void tar_backup::readBackupProfiles()
{
    QStringList profiles = stringListFromFile(QDir::homePath() + "/.tar-backup/backupProfiles");

    foreach (QString s, profiles) {
        if (s.contains("(Last backup:")) { // check if profiles are saved in old way
            QStringList a = s.split(" (Last backup: ");
            ui->list_backupProfiles->addTopLevelItem(new QTreeWidgetItem(QStringList() << a[0] << a[1].remove(')')));
        }
         else
            ui->list_backupProfiles->addTopLevelItem(new QTreeWidgetItem(s.split("#|#")));
    }
}

void tar_backup::saveBackupProfiles()
{
    QFile bpf(QDir::homePath() + "/.tar-backup/backupProfiles");
    bpf.open(QIODevice::WriteOnly);

    QByteArray tmpArr;

    for (int i=0;i<ui->list_backupProfiles->topLevelItemCount(); i++) {
        QTreeWidgetItem *t = ui->list_backupProfiles->topLevelItem(i);
        tmpArr.append(QString(t->text(0)+"#|#"+t->text(1)));
        if (i != ui->list_backupProfiles->topLevelItemCount() - 1)  // to avoid empty line at end of file
            tmpArr.append('\n');
    }
    bpf.write(tmpArr);
    bpf.close();
}

// manage profiles end //

QString tar_backup::askForPassword(const bool getPassFromFile) {
    if (getPassFromFile) {
        QString passFile;
        passFile = QFileDialog::getOpenFileName(this,"Select password file","","");

        if (!passFile.isEmpty()) {
            this->pass = "file:" + passFile;
            return this->pass;
        } else
            return QString::null;

    } else {
        bool ok;
        QString inPass = QInputDialog::getText(this,"Password for encryption",
                                     "Password", QLineEdit::Normal, "", &ok);
        if (ok && !inPass.isEmpty()) {
            this->pass = "pass:" + inPass;
            return this->pass;
        } else
            return QString::null;
    }

    return QString::null;
}

void tar_backup::on_cb_decryptOnly_clicked()
{
    if (ui->cb_decryptOnly->isChecked()) {
        ui->cb_decryptOtherDest->setEnabled(false);
        ui->cb_deleteDecryptedArch->setEnabled(false);
    } else {
        ui->cb_decryptOtherDest->setEnabled(true);
        ui->cb_deleteDecryptedArch->setEnabled(true);
    }
}

void tar_backup::on_btn_showActivityLog_clicked()
{
    if (ui->btn_showActivityLog->isChecked())
        ui->list_activityLog->setHidden(false);
    else
        ui->list_activityLog->setHidden(true);
}

