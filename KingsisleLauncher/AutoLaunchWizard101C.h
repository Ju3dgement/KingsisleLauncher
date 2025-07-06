#ifndef AUTOLAUNCHWIZARD101C_H
#define AUTOLAUNCHWIZARD101C_H

#include "ui_AutoLaunchWizard101C.h"

#include <QtWidgets/QMainWindow>
#include <QComboBox>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QFileDialog>
#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent/qtconcurrentrun.h>
#include <windows.h>
#include <TlHelp32.h>
#include "AccountInfo.h"

class AutoLaunchWizard101C : public QMainWindow
{
    Q_OBJECT

public:
    explicit AutoLaunchWizard101C(QWidget* parent = nullptr);
    ~AutoLaunchWizard101C();
    void saveJson();
    void loadJson();
    void showStyledWarning(QWidget* parent, const QString& title, const QString& text, bool warningIcon);
    
private:
    void launch();
    void bundleLaunch();
    void launchAccount(const AccountInfo& selectedAccount, const QString& game);
    void loadAccountsFromFile();
    void loadPathsFromFile();
    void loadBundlesFromFile();
    void loadSettings();
   
    Ui::AutoLaunchWizard101CClass ui;
    QJsonObject jsonData;
    QString wizardPath;
    QString piratePath;
    QList<AccountInfo> accounts;
    QMap<QString, QStringList> bundleAccounts;
};

#endif