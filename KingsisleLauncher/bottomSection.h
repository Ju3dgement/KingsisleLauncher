#ifndef BOTTOMSECTION_H
#define BOTTOMSECTION_H

#include "ui_AutoLaunchWizard101C.h"
#include "AccountInfo.h"
#include "AutoLaunchWizard101C.h"


class MiscManager : public QObject {
    Q_OBJECT
public:
    MiscManager(Ui::AutoLaunchWizard101CClass* uiPtr, QJsonObject* jsonPtr, QString* wizardPath, QString* piratePath, AutoLaunchWizard101C* parent);

public slots:
    void browse();
    void killAllClients();
    void spoof();
    void gameSelect();
	void savePathsToFile();

private:
    Ui::AutoLaunchWizard101CClass* ui;
    QJsonObject* jsonData;
    AutoLaunchWizard101C* parent;
    QString *wizardPath;
    QString *piratePath;

    bool spoofActive = false;
};

#endif