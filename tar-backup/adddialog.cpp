
// copy marazmista @ 04.2013

#include "adddialog.h"
#include "ui_adddialog.h"
#include "tar_backup.h"

#include <QSettings>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>

addDialog::addDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addDialog)
{
    ui->setupUi(this);
    loadUiIcons();
    setupSignals();
}

addDialog::addDialog(QString &profileName, QString &dest, bool &compression, QString &c_method,
                     bool &encryption, QString &e_method, QString &tarExtraParam, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addDialog)
{
    ui->setupUi(this);
    loadUiIcons();
    setupSignals();

    ui->t_dest->setText(dest);
    ui->cb_compress->setChecked(compression);
    ui->list_comp->setCurrentText(c_method);
    ui->cb_enc->setChecked(encryption);
    ui->list_enc->setCurrentText(e_method);
    ui->t_profileName->setText(profileName);
    ui->t_tarExtraParam->setText(tarExtraParam);

    QFile fList(QApplication::applicationDirPath() + "/" + profileName);
    fList.open(QIODevice::ReadOnly);
    QStringList ls;
    QTextStream ts(&fList);

    while (true)    {
        QString s = ts.readLine();
        if (s.isEmpty())
            break;
        ls.append(s);
    }
    fList.close();
    ui->list_Files->addItems(ls);
}

addDialog::~addDialog()
{
    delete ui;
}

void addDialog::setupSignals()
{
    connect(ui->cb_compress,SIGNAL(clicked(bool)),ui->list_comp,SLOT(setEnabled(bool)));
    connect(ui->cb_enc,SIGNAL(clicked(bool)),ui->list_enc,SLOT(setEnabled(bool)));
}

void addDialog::loadUiIcons() {
    ui->btn_addFolders->setIcon(QIcon::fromTheme("folder"));
    ui->btn_setDest->setIcon(QIcon::fromTheme("folder"));
    ui->btn_remove->setIcon(QIcon::fromTheme("list-remove"));
    ui->btn_save->setIcon(QIcon::fromTheme("document-save"));
    ui->btn_cancel->setIcon(QIcon::fromTheme("window-close"));
}

void addDialog::on_btn_save_clicked()
{
    if (ui->t_profileName->text().isEmpty()) {
        QMessageBox::critical(this,"Error","Profile name not set!",QMessageBox::Ok);
        return;
    }
    if (ui->t_dest->text().isEmpty()) {
        QMessageBox::critical(this,"Error","Destination not set!",QMessageBox::Ok);
        return;
    }

    QSettings pSet(QApplication::applicationDirPath()+ "/" + ui->t_profileName->text()+".ini",
                   QSettings::IniFormat);

    if (ui->t_dest->text()[ui->t_dest->text().length()-1] == '/')
        pSet.setValue("destination", ui->t_dest->text());
    else
        pSet.setValue("destination", ui->t_dest->text() + "/");

    pSet.setValue("compression", ui->cb_compress->isChecked());
    pSet.setValue("compression_method",ui->list_comp->itemText(ui->list_comp->currentIndex()));
    pSet.setValue("encryption",ui->cb_enc->isChecked());
    pSet.setValue("encryption_method",ui->list_enc->itemText(ui->list_enc->currentIndex()));
    pSet.setValue("tarExtraParam",ui->t_tarExtraParam->text());

    QFile pFileList(QApplication::applicationDirPath() + "/" + ui->t_profileName->text());
    pFileList.open(QIODevice::WriteOnly);

    QByteArray tmpArr;
    for (int i=0;i < ui->list_Files->count(); i++ ) {
        tmpArr.append(QString(ui->list_Files->item(i)->text()));
        if (i != ui->list_Files->count() - 1)
            tmpArr.append('\n');
    }
    pFileList.write(tmpArr);
    pFileList.close();

    this->backupProfileName = ui->t_profileName->text();
    this->done(1);
}

void addDialog::on_btn_addFolders_clicked()
{
    ui->list_Files->addItems(QStringList() << QFileDialog::getExistingDirectory(this,
                                                               "Select files",
                                                               "",
                                                               QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
//    QFileDialog dd;
//    dd.setFileMode(QFileDialog::DirectoryOnly);
//    if (dd.exec())
//        ui->list_Files->addItems(QStringList(dd.selectedFiles()));
}

void addDialog::on_btn_addFiles_clicked()
{
    QFileDialog fd;
    fd.setFileMode(QFileDialog::ExistingFiles);
    if (fd.exec())
        ui->list_Files->addItems(QStringList(fd.selectedFiles()));

}

void addDialog::on_btn_cancel_clicked()
{
    this->done(0);
}

void addDialog::on_btn_remove_clicked()
{
    delete ui->list_Files->currentItem();
}

void addDialog::on_btn_setDest_clicked()
{
    ui->t_dest->setText(QFileDialog::getExistingDirectory(this,
                                                               "Select files",
                                                               "",
                                                               QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
}

void addDialog::on_btn_tarManpage_clicked()
{
    QFile::copy(":/tarManPage/tar.1.txt",QApplication::applicationDirPath() + "/tarManpage.txt");
    QDesktopServices::openUrl(QUrl(QApplication::applicationDirPath() + "/tarManpage.txt"));
}
