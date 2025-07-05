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

class AutoLaunchWizard101C : public QMainWindow
{
    Q_OBJECT
    struct AccountInfo {
        QString nickname;
        QString username;
        QString password;
    };

    struct BundleInfo {
        QString bundleNickname;
        QString massBundle;
    };

public:
    explicit AutoLaunchWizard101C(QWidget* parent = nullptr);
    ~AutoLaunchWizard101C();
    
private:
    void addAccount();
    void deleteAccount();
	void addBundleAccount();
	void deleteBundleAccount();
    void launchAccount(const AccountInfo& selectedAccount, const QString& game);
    void onAccountSelected(int index);
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
    void displayTopText();
    void displayMiddleText();
    void saveUser();
    void saveBundle();
    void revealText(QPushButton* button, int index);
    void loadSettings();

    void saveJson();
    void loadJson();
    
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