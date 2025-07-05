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

class BundleManager : public QObject {
    Q_OBJECT
public:
    BundleManager(Ui::AutoLaunchWizard101CClass* uiPtr, QMap<QString, QStringList>* bundlesPtr, QJsonObject* jsonPtr, QList<AccountInfo>* accountsPtr,AutoLaunchWizard101C* parent);

public slots:
    void addBundleAccount();
    void deleteBundleAccount();
    void saveBundle();
    void displayMiddleText();
	void saveBundlesToFile();

private:
    Ui::AutoLaunchWizard101CClass* ui;
    QMap<QString, QStringList>* bundleAccounts;
    QJsonObject* jsonData;
    AutoLaunchWizard101C* parent;
    QList<AccountInfo>* accounts;
};