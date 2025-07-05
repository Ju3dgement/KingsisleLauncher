#include "AccountManager.h"
#include <QMessageBox>
#include <QJsonArray>

AccountManager::AccountManager(Ui::AutoLaunchWizard101CClass* uiPtr, QList<AccountInfo>* accountsPtr, QJsonObject* jsonPtr, AutoLaunchWizard101C* parent)
    : ui(uiPtr), accounts(accountsPtr), jsonData(jsonPtr), parent(parent){}

void AccountManager::addAccount() {
    QString nick = ui->inputNickname->text().trimmed();
    QString user = ui->inputUsername->text().trimmed();
    QString pass = ui->inputPassword->text().trimmed();

    if (nick.isEmpty() || user.isEmpty() || pass.isEmpty()) {
        parent->showStyledWarning(parent, "Input Error", "All fields must be filled", true);
        return;
    }

    for (const auto& acc : *accounts) {
        if (acc.nickname == nick) {
            parent->showStyledWarning(parent, "Duplicate", "Nickname already exists", true);
            return;
        }
    }

    QJsonObject obj;
    obj["nickname"] = nick;
    obj["username"] = user;
    obj["password"] = pass;

    QJsonArray accs = (*jsonData)["accounts"].toArray();
    accs.append(obj);
    (*jsonData)["accounts"] = accs;
    parent->saveJson();

    (*accounts).append({ nick, user, pass });
    ui->NicknameDropbox->addItem(nick);
    ui->inputNickname->clear();
    ui->inputUsername->clear();
    ui->inputPassword->clear();
}

void AccountManager::deleteAccount() {
    int index = ui->NicknameDropbox->currentIndex();
    if (index < 0 || index >= (*accounts).size()) return;

    QString nick = (*accounts)[index].nickname;
    QJsonArray accs = (*jsonData)["accounts"].toArray();
    QJsonArray updated;
    for (const QJsonValue& val : accs) {
        if (val.toObject()["nickname"].toString() != nick)
            updated.append(val);
    }
    (*jsonData)["accounts"] = updated;
    parent->saveJson();

    (*accounts).removeAt(index);
    ui->NicknameDropbox->removeItem(index);

    if (!(*accounts).isEmpty()) {
        onAccountSelected(0);
		ui->NicknameDropbox->setCurrentIndex(0);
    }
    else {
        ui->inputNickname->clear();
        ui->inputUsername->clear();
        ui->inputPassword->clear();
    }
}

void AccountManager::saveUser() {
    QString current = ui->NicknameDropbox->currentText();
    QString nick = ui->inputNickname->text().trimmed();
    QString user = ui->inputUsername->text().trimmed();
    QString pass = ui->inputPassword->text().trimmed();

    if (nick.isEmpty() || user.isEmpty() || pass.isEmpty()) {
        parent->showStyledWarning(parent, "Input Error", "All fields must be filled out before saving", true);
        return;
    }

    QJsonArray accs = (*jsonData)["accounts"].toArray();
    for (int i = 0; i < accs.size(); ++i) {
        QJsonObject obj = accs[i].toObject();
        if (obj["nickname"].toString() == current) {
            obj["nickname"] = nick;
            obj["username"] = user;
            obj["password"] = pass;
            accs[i] = obj;
            (*jsonData)["accounts"] = accs;
            parent->saveJson();

            for (AccountInfo& acc : *accounts) {
                if (acc.nickname == current) {
                    acc.nickname = nick;
                    acc.username = user;
                    acc.password = pass;
                    break;
                }
            }

            int index = ui->NicknameDropbox->findText(current);
            if (index != -1) {
                ui->NicknameDropbox->setItemText(index, nick);
                ui->NicknameDropbox->setCurrentIndex(index);
            }
            ui->SaveUserButton->hide();
            parent->showStyledWarning(parent, "Success", "Account info updated successfully", false);
            return;
        }
    }
    parent->showStyledWarning(parent, "Save Failed", "selected account could not be found in the list", false);
}

void AccountManager::displayTopText() {
    QString nickname = ui->NicknameDropbox->currentText();
    AccountInfo currentAccount;
    bool found = false;
    for (const AccountInfo& account : *accounts) {
        if (account.nickname == nickname) {
            currentAccount = account;
            found = true;
            break;
        }
    }

    if (!found) {
        ui->inputNickname->clear();
        ui->inputUsername->clear();
        ui->inputPassword->clear();
        return;
    }
    ui->inputNickname->setText(currentAccount.nickname);
    ui->inputUsername->setText(currentAccount.username);
    ui->inputPassword->setText(currentAccount.password);
}

void AccountManager::onAccountSelected(int index)
{
    if (index >= 0 && index < accounts->size()) {
        const AccountInfo& acc = (*accounts)[index];
        ui->inputNickname->setText(acc.nickname);
        ui->inputUsername->setText(acc.username);
        ui->inputPassword->setText(acc.password);
    }
}