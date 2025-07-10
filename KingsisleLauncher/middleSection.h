#ifndef MIDDLESECTION_H
#define MIDDLESECTION_H

#include "ui_AutoLaunchWizard101C.h"
#include "AccountInfo.h"
#include "AutoLaunchWizard101C.h"


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
    void changeText();
    void bundleLaunch();

private:
    Ui::AutoLaunchWizard101CClass* ui;
    QMap<QString, QStringList>* bundleAccounts;
    QJsonObject* jsonData;
    AutoLaunchWizard101C* parent;
    QList<AccountInfo>* accounts;
};

#endif