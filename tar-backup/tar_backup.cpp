
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
    loadUiIcons();
    setupProcSignals();

    QDir appHomePath(QDir::homePath() + "/.tar-backup");
    if (!appHomePath.exists())
        appHomePath.mkdir(QDir::homePath() + "/.tar-backup");
}

tar_backup::~tar_backup()
{
    saveBackupProfiles();
    delete ui;
}

void tar_backup::loadUiIcons() {
    ui->btn_run->setIcon(QIcon::fromTheme("system-run"));
    ui->btn_addProfile->setIcon(QIcon::fromTheme("list-add"));
    ui->btn_removeProfile->setIcon(QIcon::fromTheme("list-remove"));
    ui->btn_selectFileRestore->setIcon(QIcon::fromTheme("folder"));
    ui->btn_setDestRestore->setIcon(QIcon::fromTheme("folder"));
    ui->btn_abort->setIcon(QIcon::fromTheme("process-stop"));
    ui->btn_runRestore->setIcon(QIcon::fromTheme("system-run"));
    ui->btn_listMembers->setIcon(QIcon::fromTheme("system-run"));
    ui->btn_saveMembersToFile->setIcon(QIcon::fromTheme("document-save"));
    ui->btn_saveOutput->setIcon(QIcon::fromTheme("document-save"));
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
    ui->list_backupProfiles->addItems(stringListFromFile(QDir::homePath() + "/.tar-backup/backupProfiles"));
}

void tar_backup::saveBackupProfiles()
{
    QFile bpf(QDir::homePath() + "/.tar-backup/backupProfiles");
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
