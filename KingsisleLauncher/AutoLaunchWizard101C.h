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
#include <algorithm>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <windows.h>
#include <QtConcurrent/qtconcurrentrun.h>
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
    void launchAccount(const AccountInfo& selectedAccount, const QString& game);
    void loadAccountsFromFile();
    void loadPathsFromFile();
    void loadBundlesFromFile();
    void saveBundlesToFile();
    void browseWizardPath();
    void browsePiratePath();
    void savePathsToFile();
    void browse();
    void gameSelect();
    void launch();
    void bundleLaunch();
    void killAllClients();
    void spoof();
    void revealText(QPushButton* button, int index);
    void loadSettings();
    void changedText(int index);

    
    QJsonObject jsonData;
    Ui::AutoLaunchWizard101CClass ui;
    QComboBox* accountCombo;
    QString wizardPath; 
    QString piratePath; 
    QWidget* wizardPathWidget;
    QWidget* piratePathWidget;
    QComboBox* bundleCombo;
    QLineEdit* bundleNicknameEdit;
    QLineEdit* bundleMassNicknamesEdit;
    QPushButton* bundleLaunchButton;
    QList<AccountInfo> accounts;
    QMap<QString, QStringList> bundleAccounts;
    bool spoofActive = false;
};

#endif