#ifndef TOPSECTION_H
#define TOPSECTION_H

#include "ui_AutoLaunchWizard101C.h"
#include "AccountInfo.h"
#include "AutoLaunchWizard101C.h"

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
    void changeText();
    void revealText(QPushButton* button, int index);
    void launch();

private:
    Ui::AutoLaunchWizard101CClass* ui;
    QList<AccountInfo>* accounts;
    QJsonObject* jsonData;
    AutoLaunchWizard101C* parent;
};

#endif