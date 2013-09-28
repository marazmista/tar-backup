#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>
#include <QString>
#include <QListWidget>

namespace Ui {
class addDialog;
}

class addDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit addDialog(QWidget *parent = 0);
    explicit addDialog(QString &profileName,QString &dest,bool &compression, QString &c_method,
                       bool &encryption, QString &e_method, QString &tarExtraParam, bool &excludeCaches,
                       bool &excludeVcs, bool &excludeBackups, bool &oneFilesystem, bool &showTotals, bool &preservePermissions,
                       bool &passFromFile, QWidget *parent = 0);
    ~addDialog();
    QString backupProfileName;
    
    void setupSignals();

private slots:
    void deleteKeyOnSourcesList();
    void on_btn_save_clicked();
    void deleteKeyOnPatternsList();
    void refreshPatternsSummary();
    void on_btn_addFolders_clicked();
    void on_btn_addFiles_clicked();
    void on_btn_cancel_clicked();
    void on_btn_remove_clicked();
    void on_btn_setDest_clicked();
    void on_btn_tarManpage_clicked();
    void on_btn_addPattern_clicked();
    void on_btn_modifyPattern_clicked();
    void on_btn_removePattern_clicked();
    void on_btn_customPath_clicked();
    void on_btn_importFromFile_clicked();
    void on_btn_addExcludeFiles_clicked();
    void on_btn_addExclupdeFolders_clicked();

    void on_pushButton_clicked();

private:
    Ui::addDialog *ui;
    QByteArray listToByteArray(const QListWidget *list);
    QStringList stringListFromFile(const QString &fPath);
};

#endif // ADDDIALOG_H
