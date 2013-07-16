
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
#include <QShortcut>
#include <QInputDialog>

addDialog::addDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addDialog)
{
    ui->setupUi(this);
    loadUiIcons();
    setupSignals();
}

QStringList addDialog::stringListFromFile(const QString &fPath)
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

addDialog::addDialog(QString &profileName, QString &dest, bool &compression, QString &c_method,
                     bool &encryption, QString &e_method, QString &tarExtraParam, bool &excludeCaches,
                     bool &excludeVcs, bool &excludeBackups, bool &oneFilesystem, bool &showTotals, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addDialog)
{
    ui->setupUi(this);
    loadUiIcons();
    setupSignals();

    ui->t_dest->setText(dest);
    ui->cb_compress->setChecked(compression);
    ui->list_comp->setCurrentIndex(ui->list_comp->findText(c_method));
    ui->cb_enc->setChecked(encryption);
    ui->list_enc->setCurrentIndex(ui->list_enc->findText(e_method));
    ui->t_profileName->setText(profileName);
    ui->t_tarExtraParam->setText(tarExtraParam);
    ui->cb_excludeCaches->setChecked(excludeCaches);
    ui->cb_oneFilesystem->setChecked(oneFilesystem);
    ui->cb_showTotals->setChecked(showTotals);
    ui->cb_excludeVcs->setChecked(excludeVcs);
    ui->cb_excludeBackups->setChecked(excludeBackups);

    ui->list_Files->addItems(stringListFromFile(QApplication::applicationDirPath() + "/" + profileName));
    ui->list_patterns->addItems(stringListFromFile(QApplication::applicationDirPath() + "/" + profileName + "-excludePatterns"));
    refreshPatternsSummary();
}

addDialog::~addDialog()
{
    delete ui;
}

void addDialog::setupSignals()
{
    connect(ui->cb_compress,SIGNAL(clicked(bool)),ui->list_comp,SLOT(setEnabled(bool)));
    connect(ui->cb_enc,SIGNAL(clicked(bool)),ui->list_enc,SLOT(setEnabled(bool)));
    connect(ui->list_patterns,SIGNAL(itemChanged(QListWidgetItem*)),this,SLOT(refreshPatternsSummary()));

    QShortcut *qs = new QShortcut(Qt::Key_Delete,ui->list_Files);
    connect(qs,SIGNAL(activated()),this,SLOT(deleteKeyOnSourcesList()));
    QShortcut *qp = new QShortcut(Qt::Key_Delete,ui->list_patterns);
    connect(qp,SIGNAL(activated()),this,SLOT(deleteKeyOnPatternsList()));
}

void addDialog::deleteKeyOnSourcesList() {
    ui->btn_remove->click();
}

void addDialog::loadUiIcons() {
    ui->btn_addFolders->setIcon(QIcon::fromTheme("folder"));
    ui->btn_setDest->setIcon(QIcon::fromTheme("folder"));
    ui->btn_remove->setIcon(QIcon::fromTheme("list-remove"));
    ui->btn_save->setIcon(QIcon::fromTheme("document-save"));
    ui->btn_cancel->setIcon(QIcon::fromTheme("window-close"));
}

QByteArray addDialog::listToByteArray(const QListWidget *list)
{
    QByteArray tmpArr;
    for (int i=0;i < list->count(); i++ ) {
        tmpArr.append(QString(list->item(i)->text()));
        if (i != list->count() - 1)
            tmpArr.append('\n');
    }

    return tmpArr;
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
    pSet.setValue("excludeCaches",ui->cb_excludeCaches->isChecked());
    pSet.setValue("excludeVCS", ui->cb_excludeVcs->isChecked());
    pSet.setValue("excludeBackups",ui->cb_excludeBackups->isChecked());
    pSet.setValue("oneFilesystem",ui->cb_oneFilesystem->isChecked());
    pSet.setValue("showTotals",ui->cb_showTotals->isChecked());

    QFile pFileList(QApplication::applicationDirPath() + "/" + ui->t_profileName->text());
    pFileList.open(QIODevice::WriteOnly);
    pFileList.write(listToByteArray(ui->list_Files));
    pFileList.close();

    QFile pPattersList(QApplication::applicationDirPath() + "/" +ui->t_profileName->text()+"-excludePatterns");
    pPattersList.open(QIODevice::WriteOnly);
    pPattersList.write(listToByteArray(ui->list_patterns));
    pPattersList.close();

    this->backupProfileName = ui->t_profileName->text();
    this->done(1);
}

void addDialog::on_btn_cancel_clicked()
{
    this->done(0);
}


// Sources tab //

void addDialog::on_btn_addFolders_clicked()
{
    QString s = QFileDialog::getExistingDirectory(this,"Add directory", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!s.isEmpty())
        ui->list_Files->addItem(s);
}

void addDialog::on_btn_addFiles_clicked()
{
    QFileDialog fd;
    fd.setFileMode(QFileDialog::ExistingFiles);
    if (fd.exec())
        ui->list_Files->addItems(QStringList(fd.selectedFiles()));

}

void addDialog::on_btn_remove_clicked()
{
    delete ui->list_Files->currentItem();
}

void addDialog::on_btn_setDest_clicked()
{
    QString s = QFileDialog::getExistingDirectory(this,"Select destination", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!s.isEmpty())
        ui->t_dest->setText(s);
}


// settings tab //

void addDialog::on_btn_tarManpage_clicked()
{
    QFile::copy(":/tarManPage/tar.1.txt",QApplication::applicationDirPath() + "/tarManpage.txt");
    QDesktopServices::openUrl(QUrl(QApplication::applicationDirPath() + "/tarManpage.txt"));
}


// patterns tab //

void addDialog::on_btn_addPattern_clicked()
{
    bool ok;
    QString pattern = QInputDialog::getText(this,"Add pattern", "New pattern",QLineEdit::Normal,"",&ok);

    if (ok && !pattern.isEmpty()) {
        ui->list_patterns->addItem("\"" + pattern + "\"");
    }
    refreshPatternsSummary();
}

void addDialog::on_btn_modifyPattern_clicked()
{
    if (ui->list_patterns->currentItem() != 0) {
        ui->list_patterns->currentItem()->setFlags(ui->list_patterns->currentItem()->flags() | Qt::ItemIsEditable);
        ui->list_patterns->editItem(ui->list_patterns->currentItem());
    }
}

void addDialog::on_btn_removePattern_clicked()
{
    delete ui->list_patterns->currentItem();
    refreshPatternsSummary();
}

void addDialog::refreshPatternsSummary() {
    QString patternsSummary;
    for (int i=0;i < ui->list_patterns->count(); i++ ) {
        patternsSummary += "--exclude=" +ui->list_patterns->item(i)->text()+" ";
    }
    ui->t_patternsSummary->setText(patternsSummary);
}

void addDialog::deleteKeyOnPatternsList() {
    ui->btn_removePattern->click();
}
