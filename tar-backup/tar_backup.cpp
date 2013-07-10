
// copy marazmista @ 04.2013

#include "tar_backup.h"
#include "ui_tar_backup.h"
#include "adddialog.h"

#include <QSettings>
#include <QTextStream>

tar_backup::tar_backup(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::tar_backup)
{
    ui->setupUi(this);
    readBackupProfiles();
    loadUiIcons();
}

tar_backup::~tar_backup()
{
    delete ui;
}

void tar_backup::loadUiIcons() {
    ui->btn_run->setIcon(QIcon::fromTheme("system-run"));
    ui->btn_addProfile->setIcon(QIcon::fromTheme("list-add"));
//    ui->btn_modifyProfile->setIcon(QIcon::fromTheme());
    ui->btn_removeProfile->setIcon(QIcon::fromTheme("list-remove"));
    ui->btn_selectFileRestore->setIcon(QIcon::fromTheme("folder"));
    ui->btn_setDestRestore->setIcon(QIcon::fromTheme("folder"));
    ui->btn_abort->setIcon(QIcon::fromTheme("process-stop"));
    ui->btn_runRestore->setIcon(QIcon::fromTheme("system-run"));
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

// manage profiles end //


void tar_backup::enableButtons(bool val)
{
    ui->btn_run->setEnabled(val);
    ui->btn_runRestore->setEnabled(val);
}
