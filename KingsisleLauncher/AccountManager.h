#pragma once

#include <QObject>
#include <QVector>
#include <QJsonObject>
#include <QList>
#include "ui_AutoLaunchWizard101C.h"
#include "AccountInfo.h"
#include <QFile>
#include "AutoLaunchWizard101C.h"
#include <QMessageBox>
class AccountManager : public QObject {
    Q_OBJECT
public:
    AccountManager(Ui::AutoLaunchWizard101CClass* uiPtr, QList<AccountInfo>* accountsPtr, QJsonObject* jsonPtr, AutoLaunchWizard101C* parent);

public slots:
    void addAccount();
    void deleteAccount();
    void saveUser();
    void displayTopText();
	void onAccountSelected(int index);

private:
    Ui::AutoLaunchWizard101CClass* ui;
    QList<AccountInfo>* accounts;
    QJsonObject* jsonData;
    AutoLaunchWizard101C* parent;
};