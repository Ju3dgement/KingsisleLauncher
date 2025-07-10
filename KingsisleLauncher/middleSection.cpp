#include "middleSection.h"

BundleManager::BundleManager(Ui::AutoLaunchWizard101CClass* uiPtr, QMap<QString, QStringList>* bundlesPtr, QJsonObject* jsonPtr, QList<AccountInfo>* accountsPtr, AutoLaunchWizard101C* parent)
    : ui(uiPtr), bundleAccounts(bundlesPtr), jsonData(jsonPtr), accounts(accountsPtr), parent(parent) {
}

void BundleManager::addBundleAccount() {
    QString nickname = ui->inputBundleNickname->text().trimmed();
    QString massNick = ui->inputMassNicknames->text().trimmed();

    if (nickname.isEmpty() || massNick.isEmpty()) {
        parent->showStyledWarning(parent, "Input Error", "Both fields must be filled", true);
        return;
    }

    if (bundleAccounts->contains(nickname)) {
        parent->showStyledWarning(parent, "Duplicate", "Bundle nickname already exists", true);
        return;
    }

    QStringList nicknames = massNick.split("/", Qt::SkipEmptyParts);
    for (QString& name : nicknames) {
        name = name.trimmed();
        bool exists = std::any_of(accounts->begin(), accounts->end(), [&](const AccountInfo& acc) {
            return acc.nickname == name;
            });
        if (!exists) {
            parent->showStyledWarning(parent, "Invalid Nickname", "Nickname not found: " + name, true);
            return;
        }
    }
    (*bundleAccounts)[nickname] = nicknames;
    ui->BundleNicknameDropbox->addItem(nickname);
    saveBundlesToFile();
}

void BundleManager::deleteBundleAccount() {
    QString bundle = ui->BundleNicknameDropbox->currentText();
    if (bundleAccounts->isEmpty()) return;

    bundleAccounts->remove(bundle);
    int index = ui->BundleNicknameDropbox->findText(bundle);
    if (index >= 0)
        ui->BundleNicknameDropbox->removeItem(index);

    QJsonObject bundlesObj = (*jsonData)["bundles"].toObject();
    if (bundlesObj.contains(bundle)) {
        bundlesObj.remove(bundle);
        (*jsonData)["bundles"] = bundlesObj;
        parent->saveJson();
    }

    if (!bundleAccounts->isEmpty()) {
        QString firstKey = bundleAccounts->firstKey();
        const QStringList& nicks = bundleAccounts->value(firstKey);

        ui->inputBundleNickname->setText(firstKey);
        ui->inputMassNicknames->setText(nicks.join("/"));
        ui->BundleNicknameDropbox->setCurrentText(firstKey);
    }
    else {
        ui->inputBundleNickname->clear();
        ui->inputMassNicknames->clear();
        ui->BundleNicknameDropbox->setCurrentIndex(0);
    }

    saveBundlesToFile();
}


void BundleManager::saveBundle() {
    QString currentBundleNickname = ui->BundleNicknameDropbox->currentText();
    QString bundleNickname = ui->inputBundleNickname->text().trimmed();
    QString massNicknames = ui->inputMassNicknames->text().trimmed();

    if (bundleNickname.isEmpty() || massNicknames.isEmpty()) {
        parent->showStyledWarning(parent, "Input Error", "Both fields must be filled out before saving", true);
        return;
    }

    parent->loadJson();

    QJsonObject bundlesObj = (*jsonData)["bundles"].toObject();
    if (!bundlesObj.contains(currentBundleNickname)) {
        parent->showStyledWarning(parent, "Save Failed", "The selected bundle could not be found", true);
        return;
    }

    QStringList massNickList = massNicknames.split('/', Qt::SkipEmptyParts);
    bundlesObj.remove(currentBundleNickname);

    QJsonArray newNickArray;
    for (const QString& nick : massNickList)
        newNickArray.append(nick);

    bundlesObj[bundleNickname] = newNickArray;
    (*jsonData)["bundles"] = bundlesObj;
    parent->saveJson();
    bundleAccounts->remove(currentBundleNickname);
    (*bundleAccounts)[bundleNickname] = massNickList;

    int index = ui->BundleNicknameDropbox->findText(currentBundleNickname);
    if (index != -1) {
        ui->BundleNicknameDropbox->setItemText(index, bundleNickname);
        ui->BundleNicknameDropbox->setCurrentIndex(index);
    }
    parent->showStyledWarning(parent, "Success", "Bundle info updated successfully", false);
}

void BundleManager::displayMiddleText() {
    QString nickname = ui->BundleNicknameDropbox->currentText();

    if (bundleAccounts->contains(nickname)) {
        ui->inputBundleNickname->setText(nickname);
        ui->inputMassNicknames->setText((*bundleAccounts)[nickname].join("/"));
    }
    else {
        ui->inputBundleNickname->clear();
        ui->inputMassNicknames->clear();
    }
}

void BundleManager::saveBundlesToFile() {
    QJsonObject bundles;
    for (auto it = bundleAccounts->begin(); it != bundleAccounts->end(); ++it) {
        QJsonArray array;
        for (const QString& nick : it.value())
            array.append(nick);
        bundles[it.key()] = array;
    }
    (*jsonData)["bundles"] = bundles;
    parent->saveJson();
}

void BundleManager::changeText() {
    QString bundleNickname = ui->inputBundleNickname->text().trimmed();
    QString massNicknames = ui->inputMassNicknames->text().trimmed();

    if (bundleNickname.isEmpty() || massNicknames.isEmpty() || ui->BundleNicknameDropbox->count() == 0){
        ui->SaveBundleButton->hide();
        return;
	}

    parent->loadJson();
    QJsonObject bundlesObj = (*jsonData)["bundles"].toObject();
    bool matchFound = false;

    if (bundlesObj.contains(bundleNickname)) {
        matchFound = true;
        QJsonArray storedArray = bundlesObj[bundleNickname].toArray();
        QStringList storedList;
        for (const QJsonValue& val : storedArray)
            storedList.append(val.toString());

        QString storedMassNicknames = storedList.join("/");

        if (storedMassNicknames != massNicknames) {
            ui->SaveBundleButton->show();
        }
        else {
            ui->SaveBundleButton->hide();
        }
    }
    if (!matchFound) {
        ui->SaveBundleButton->show();
    }
}

void BundleManager::bundleLaunch() {
    QString bundleName = ui->BundleNicknameDropbox->currentText();
    if (bundleName.isEmpty() || !(*bundleAccounts).contains(bundleName)) return;

    QString game = ui->GameDropbox->currentText();
    QStringList& nicknames = (*bundleAccounts)[bundleName];

    for (QString& nick : nicknames) {
        auto findCreds = std::find_if(accounts->begin(), accounts->end(), [&](AccountInfo& a) {
            return a.nickname == nick;
            });
        if (findCreds != accounts->end()) {
            parent->launchAccount(*findCreds, game);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // stagger 
        }
    }
}