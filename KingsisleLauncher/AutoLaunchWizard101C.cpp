// Yes I know this is bad coding gonna seperate this into seperate classes eventually
#include "AutoLaunchWizard101C.h"
#include "ui_AutoLaunchWizard101C.h"
#include <windows.h>
#include <QtConcurrent/qtconcurrentrun.h>
#include <TlHelp32.h>

AutoLaunchWizard101C::AutoLaunchWizard101C(QWidget* parent)
{
    ui.setupUi(this);
    setWindowTitle("Ju3dge Launcher");
    setWindowIcon(QIcon("images/Ju3dge.ico"));
    ui.GameDropbox->addItem("Wizard101");
    ui.GameDropbox->addItem("Pirate101");
    loadAccountsFromFile();
    loadPathsFromFile();
    loadBundlesFromFile();
    loadSettings();
    ui.inputWizardPath->setReadOnly(true);
    ui.SaveUserButton->setStyleSheet("background-color: green; color: white;");
    ui.SaveBundleButton->setStyleSheet("background-color: green; color: white;");
    connect(ui.AddAccountButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::addAccount);
    connect(ui.DeleteAccountButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::deleteAccount);
    connect(ui.AddBundleButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::addBundleAccount);
    connect(ui.DeleteBundleButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::deleteBundleAccount);
    connect(ui.BrowseButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::browse);
    connect(ui.LaunchButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::launch);
    connect(ui.BundleLaunchButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::bundleLaunch);
    connect(ui.killAllButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::killAllClients);    
    connect(ui.SpoofButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::spoof);

    connect(ui.GameDropbox, &QComboBox::activated, this, &AutoLaunchWizard101C::gameSelect);
    connect(ui.NicknameDropbox, &QComboBox::activated, this, &AutoLaunchWizard101C::displayTopText);
    connect(ui.BundleNicknameDropbox, &QComboBox::activated, this, &AutoLaunchWizard101C::displayMiddleText);

    connect(ui.SaveUserButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::saveUser);
    connect(ui.SaveBundleButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::saveBundle);
    connect(ui.UsernameReveal, &QPushButton::clicked, this, [=]() {revealText(ui.UsernameReveal, 0);});
    connect(ui.PasswordReveal, &QPushButton::clicked, this, [=]() {revealText(ui.PasswordReveal, 1);});
}

AutoLaunchWizard101C::~AutoLaunchWizard101C() {}

void AutoLaunchWizard101C::saveBundle() {
    QString currentBundleNickname = ui.BundleNicknameDropbox->currentText();
    QString bundleNickname = ui.inputBundleNickname->text().trimmed();
    QString massNicknames = ui.inputMassNicknames->text().trimmed();

    if (bundleNickname.isEmpty() || massNicknames.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Both fields must be filled out before saving");
        return;
    }

    loadJson();

    QJsonObject bundlesObj = jsonData["bundles"].toObject();
    if (!bundlesObj.contains(currentBundleNickname)) {
        QMessageBox::warning(this, "Save Failed", "The selected bundle could not be found.");
        return;
    }

    QStringList massNickList = massNicknames.split('/', Qt::SkipEmptyParts);
    bundlesObj.remove(currentBundleNickname);

    QJsonArray newNickArray;
    for (const QString& nick : massNickList)
        newNickArray.append(nick);

    bundlesObj[bundleNickname] = newNickArray;
    jsonData["bundles"] = bundlesObj;
    saveJson(); 
    bundleAccounts.remove(currentBundleNickname);
    bundleAccounts[bundleNickname] = massNickList;

    int index = ui.BundleNicknameDropbox->findText(currentBundleNickname);
    if (index != -1) {
        ui.BundleNicknameDropbox->setItemText(index, bundleNickname);
        ui.BundleNicknameDropbox->setCurrentIndex(index);
    }

    QMessageBox::information(this, "Success", "Bundle info updated successfully.");
}


void AutoLaunchWizard101C::displayMiddleText() {
    QString nickname = ui.BundleNicknameDropbox->currentText();

    if (bundleAccounts.contains(nickname)) {
        ui.inputBundleNickname->setText(nickname);
        ui.inputMassNicknames->setText(bundleAccounts[nickname].join("/"));
    }
    else {
        ui.inputBundleNickname->clear();
        ui.inputMassNicknames->clear();
    }
}

void AutoLaunchWizard101C::displayTopText() {
    QString nickname = ui.NicknameDropbox->currentText();
    AccountInfo currentAccount;
    bool found = false;
    for (const AccountInfo& account : accounts) {
        if (account.nickname == nickname) {
            currentAccount = account;
            found = true;
            break;
        }
    }

    if (!found) {
        ui.inputNickname->clear();
        ui.inputUsername->clear();
        ui.inputPassword->clear();
        return;
    }

    ui.inputNickname->setText(currentAccount.nickname);
    ui.inputUsername->setText(currentAccount.username);
    ui.inputPassword->setText(currentAccount.password);
}

void AutoLaunchWizard101C::killAllClients() {
    QString targetName = "WizardGraphicalClient.exe";

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        QMessageBox::warning(this, "Error", "Failed to create process snapshot.");
        return;
    }

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    bool foundAny = false;

    if (Process32FirstW(snapshot, &entry)) {
        do {
            QString processName = QString::fromWCharArray(entry.szExeFile);

            if (processName.compare(targetName, Qt::CaseInsensitive) == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
                if (hProcess) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                    foundAny = true;
                }
            }

        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);

    if (foundAny) {
        QMessageBox::information(this, "Done", "All Wizard101 clients have been terminated.");
    }
    else {
        QMessageBox::information(this, "Info", "No Wizard101 clients are running.");
    }
}

void AutoLaunchWizard101C::spoof() {
    // Work in Progress
    if (!spoofActive) {
        spoofActive = !spoofActive;
        ui.SpoofButton->setStyleSheet("background-color: green; color: white;");
        ui.SpoofButton->setText("Spoof (ON)");
    }
    else {
        spoofActive = !spoofActive;
        ui.SpoofButton->setStyleSheet("background-color: red; color: white;");
        ui.SpoofButton->setText("Spoof (OFF)");
    }
}
void AutoLaunchWizard101C::launchAccount(const AccountInfo& selectedAccount, const QString& game) {
    QString exePathFixed = (game == "Wizard101") ? wizardPath : piratePath;
    if (!exePathFixed.endsWith("/Bin", Qt::CaseInsensitive)) {
        exePathFixed += "/Bin";
    }
    QString exeName = (game == "Wizard101") ? "WizardGraphicalClient.exe" : "Pirate.exe";
    QString fullPath = exePathFixed + "/" + exeName;

    if (!QFile::exists(fullPath)) {
        QMessageBox::warning(this, "Launch Failed", QString("Game executable not found at:\n%1").arg(fullPath));
        return;
    }

    QString cmd = QString("\"%1\" -L login.us.%2.com 12000").arg(fullPath, game.toLower());

    QtConcurrent::run([=]() {
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;

        BOOL success = CreateProcessW(
            nullptr,
            (LPWSTR)cmd.utf16(),
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            (LPCWSTR)exePathFixed.utf16(),
            &si,
            &pi
        );

        if (!success) {
            QMetaObject::invokeMethod(this, [=]() {
                QMessageBox::warning(this, "Launch Failed", QString("Could not launch client for %1.").arg(selectedAccount.nickname));
                }, Qt::QueuedConnection);
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        HWND hwnd = nullptr;
        //  window by title
        for (int i = 0; i < 50 && hwnd == nullptr; ++i) {
            hwnd = FindWindowW(nullptr, (LPCWSTR)game.utf16());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (hwnd) {
            for (QChar character : selectedAccount.username)
                SendMessageW(hwnd, WM_CHAR, character.unicode(), 0); // username

            SendMessageW(hwnd, WM_CHAR, 9, 0); // Tab

            for (QChar character : selectedAccount.password)
                SendMessageW(hwnd, WM_CHAR, character.unicode(), 0); // password

            SendMessageW(hwnd, WM_CHAR, 13, 0); // Enter

            std::wstring windowTitle = selectedAccount.nickname.toStdWString();
            SetWindowTextW(hwnd, windowTitle.c_str());
        }
        else {
            QMetaObject::invokeMethod(this, [=]() {
                QMessageBox::warning(this, "Window Not Found", QString("Could not find game window %1 after launch.").arg(selectedAccount.nickname));
                }, Qt::QueuedConnection);
        }
        });
}

void AutoLaunchWizard101C::launch() {
    QString nickname = ui.NicknameDropbox->currentText();
    if (nickname.isEmpty()) return;

    auto findCreds = std::find_if(accounts.begin(), accounts.end(), [&](const AccountInfo& a) {
        return a.nickname == nickname;
        });

    if (findCreds == accounts.end()) return;

    QString game = ui.GameDropbox->currentText();
    launchAccount(*findCreds, game);
}

void AutoLaunchWizard101C::bundleLaunch() {
    QString bundleName = ui.BundleNicknameDropbox->currentText();
    if (bundleName.isEmpty() || !bundleAccounts.contains(bundleName)) return;

    QString game = ui.GameDropbox->currentText(); 
    const QStringList& nicknames = bundleAccounts[bundleName];

    for (const QString& nick : nicknames) {
        auto findCreds = std::find_if(accounts.begin(), accounts.end(), [&](const AccountInfo& a) {
            return a.nickname == nick;
            });
        if (findCreds != accounts.end()) {
			launchAccount(*findCreds, game);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // stagger launches
        }
    }
}

void AutoLaunchWizard101C::browse(){
    if (ui.GameDropbox->currentText() == "Wizard101") {
        browseWizardPath();
    } else if (ui.GameDropbox->currentText() == "Pirate101") {
        browsePiratePath();
    }
}

void AutoLaunchWizard101C::browseWizardPath() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Wizard101 Path");
    if (!dir.isEmpty()) {
        ui.inputWizardPath->setText(dir);
        wizardPath = dir;
        savePathsToFile();
    }
}

void AutoLaunchWizard101C::browsePiratePath() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Pirate101 Path");
    if (!dir.isEmpty()) {
        ui.inputWizardPath->setText(dir);
        piratePath = dir;
        savePathsToFile();
    }
}

void AutoLaunchWizard101C::gameSelect() {
    QString game = ui.GameDropbox->currentText();
    if (game == "Wizard101") {
		ui.Wizard101PathLabel->setText("Wizard101 Path");
        ui.inputWizardPath->setText(wizardPath);

    } else if (game == "Pirate101") {
        ui.Wizard101PathLabel->setText("Pirate101 Path");
        ui.inputWizardPath->setText(piratePath);
	}
}

void AutoLaunchWizard101C::addBundleAccount() {
    QString nickname = ui.inputBundleNickname->text().trimmed();
    QString massNick = ui.inputMassNicknames->text().trimmed();

    if (nickname.isEmpty() || massNick.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Both fields must be filled.");
        return;
    }

    if (bundleAccounts.contains(nickname)) {
        QMessageBox::warning(this, "Duplicate", "Bundle nickname already exists.");
        return;
    }

    QStringList nicknames = massNick.split("/", Qt::SkipEmptyParts);
    for (QString& name : nicknames) {
        name = name.trimmed();
        bool exists = std::any_of(accounts.begin(), accounts.end(), [&](const AccountInfo& acc) {
            return acc.nickname == name;
            });
        if (!exists) {
            QMessageBox::warning(this, "Invalid Nickname", "Nickname not found: " + name);
            return;
        }
    }
    bundleAccounts[nickname] = nicknames;
    ui.BundleNicknameDropbox->addItem(nickname);
    saveBundlesToFile();
}

void AutoLaunchWizard101C::deleteBundleAccount() {
    QString bundle = ui.BundleNicknameDropbox->currentText();
    if (!bundleAccounts.contains(bundle)) {
        QMessageBox::warning(this, "Error", "No such bundle.");
        return;
    }

	bundleAccounts.remove(bundle);
    int index = ui.BundleNicknameDropbox->findText(bundle);
    if (index >= 0)
        ui.BundleNicknameDropbox->removeItem(index);
    saveBundlesToFile();
}

void AutoLaunchWizard101C::onAccountSelected(int index)
{
    if (index >= 0 && index < accounts.size()) {
        const AccountInfo& acc = accounts[index];
        ui.inputNickname->setText(acc.nickname);
        ui.inputUsername->setText(acc.username);
        ui.inputPassword->setText(acc.password);
    }
}

// =======================================================================================================

void AutoLaunchWizard101C::loadJson() {
    QFile file("information/data.json");
    if (!file.exists()) {
        file.open(QIODevice::WriteOnly);
        QJsonObject root;
        root["accounts"] = QJsonArray();
        root["paths"] = QJsonObject{ {"wizard", ""}, {"pirate", ""} };
        root["bundles"] = QJsonObject();
        root["settings"] = QJsonArray{ "Off", "Off" };
        file.write(QJsonDocument(root).toJson());
        file.close();
    }
    file.open(QIODevice::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    jsonData = doc.object();
    file.close();
}

void AutoLaunchWizard101C::saveJson() {
    QFile file("information/data.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(QJsonDocument(jsonData).toJson());
        file.close();
    }
}

void AutoLaunchWizard101C::loadAccountsFromFile() {
    loadJson();
    accounts.clear();
    ui.NicknameDropbox->clear();
    QJsonArray accs = jsonData["accounts"].toArray();
    for (auto val : accs) {
        QJsonObject obj = val.toObject();
        AccountInfo acc{ obj["nickname"].toString(), obj["username"].toString(), obj["password"].toString() };
        accounts.append(acc);
        ui.NicknameDropbox->addItem(acc.nickname);
    }
}

void AutoLaunchWizard101C::saveUser() {
    QString current = ui.NicknameDropbox->currentText();
    QString nick = ui.inputNickname->text().trimmed();
    QString user = ui.inputUsername->text().trimmed();
    QString pass = ui.inputPassword->text().trimmed();

    if (nick.isEmpty() || user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields must be filled out before saving");
        return;
    }

    QJsonArray accs = jsonData["accounts"].toArray();
    for (int i = 0; i < accs.size(); ++i) {
        QJsonObject obj = accs[i].toObject();
        if (obj["nickname"].toString() == current) {
            obj["nickname"] = nick;
            obj["username"] = user;
            obj["password"] = pass;
            accs[i] = obj;
            jsonData["accounts"] = accs;
            saveJson();

            for (AccountInfo& acc : accounts) {
                if (acc.nickname == current) {
                    acc.nickname = nick;
                    acc.username = user;
                    acc.password = pass;
                    break;
                }
            }

            int index = ui.NicknameDropbox->findText(current);
            if (index != -1) {
                ui.NicknameDropbox->setItemText(index, nick);
                ui.NicknameDropbox->setCurrentIndex(index);
            }
            QMessageBox::information(this, "Success", "Account info updated successfully.");
            return;
        }
    }
    QMessageBox::warning(this, "Save Failed", "The selected account could not be found in the list.");
}

void AutoLaunchWizard101C::addAccount() {
    QString nick = ui.inputNickname->text().trimmed();
    QString user = ui.inputUsername->text().trimmed();
    QString pass = ui.inputPassword->text().trimmed();

    if (nick.isEmpty() || user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields must be filled out.");
        return;
    }

    for (const AccountInfo& acc : accounts) {
        if (acc.nickname == nick) {
            QMessageBox::warning(this, "Duplicate", "Nickname already exists.");
            return;
        }
    }

    QJsonObject obj;
    obj["nickname"] = nick;
    obj["username"] = user;
    obj["password"] = pass;

    QJsonArray accs = jsonData["accounts"].toArray();
    accs.append(obj);
    jsonData["accounts"] = accs;
    saveJson();

    accounts.append({ nick, user, pass });
    ui.NicknameDropbox->addItem(nick);
    ui.inputNickname->clear();
    ui.inputUsername->clear();
    ui.inputPassword->clear();
}

void AutoLaunchWizard101C::deleteAccount() {
    int index = ui.NicknameDropbox->currentIndex();
    if (index < 0 || index >= accounts.size()) {
        QMessageBox::warning(this, "Delete Error", "No account selected.");
        return;
    }

    QString nick = accounts[index].nickname;
    QJsonArray accs = jsonData["accounts"].toArray();
    QJsonArray updated;
    for (const QJsonValue& val : accs) {
        if (val.toObject()["nickname"].toString() != nick)
            updated.append(val);
    }
    jsonData["accounts"] = updated;
    saveJson();

    accounts.removeAt(index);
    ui.NicknameDropbox->removeItem(index);
    if (!accounts.isEmpty()) {
        ui.NicknameDropbox->setCurrentIndex(0);
        onAccountSelected(0);
    }
    else {
        ui.inputNickname->clear();
        ui.inputUsername->clear();
        ui.inputPassword->clear();
    }
}

void AutoLaunchWizard101C::loadPathsFromFile() {
    QJsonObject paths = jsonData["paths"].toObject();
    wizardPath = paths["wizard"].toString();
    piratePath = paths["pirate"].toString();
    QString game = ui.GameDropbox->currentText();
    if (game == "Wizard101") {
        ui.inputWizardPath->setText(wizardPath);
    }
    else if (game == "Pirate101"){
        ui.inputWizardPath->setText(piratePath);
    }
}

void AutoLaunchWizard101C::savePathsToFile() {
    QJsonObject paths;
    paths["wizard"] = wizardPath;
    paths["pirate"] = piratePath;
    jsonData["paths"] = paths;
    saveJson();
}

void AutoLaunchWizard101C::loadBundlesFromFile() {
    ui.BundleNicknameDropbox->clear();
    bundleAccounts.clear();
    QJsonObject bundles = jsonData["bundles"].toObject();
    for (const QString& key : bundles.keys()) {
        QStringList nicks;
        for (const QJsonValue& v : bundles[key].toArray()) {
            nicks.append(v.toString());
        }
        bundleAccounts[key] = nicks;
        ui.BundleNicknameDropbox->addItem(key);
    }
}

void AutoLaunchWizard101C::saveBundlesToFile() {
    QJsonObject bundles;
    for (auto it = bundleAccounts.begin(); it != bundleAccounts.end(); ++it) {
        QJsonArray array;
        for (const QString& nick : it.value())
            array.append(nick);
        bundles[it.key()] = array;
    }
    jsonData["bundles"] = bundles;
    saveJson();
}

void AutoLaunchWizard101C::loadSettings() {
    QJsonArray settings = jsonData["settings"].toArray();
    QString line1 = settings.size() > 0 ? settings[0].toString() : "Off";
    QString line2 = settings.size() > 1 ? settings[1].toString() : "Off";

    if (line1.compare("On", Qt::CaseInsensitive) == 0) {
        ui.inputUsername->setEchoMode(QLineEdit::Normal);
        ui.UsernameReveal->setStyleSheet("background-color: red; color: white;");
        ui.UsernameReveal->setText("Off");
    }
    else {
        ui.inputUsername->setEchoMode(QLineEdit::Password);
        ui.UsernameReveal->setStyleSheet("background-color: green; color: white;");
        ui.UsernameReveal->setText("On");
    }

    if (line2.compare("On", Qt::CaseInsensitive) == 0) {
        ui.inputPassword->setEchoMode(QLineEdit::Normal);
        ui.PasswordReveal->setStyleSheet("background-color: red; color: white;");
        ui.PasswordReveal->setText("Off");
    }
    else {
        ui.inputPassword->setEchoMode(QLineEdit::Password);
        ui.PasswordReveal->setStyleSheet("background-color: green; color: white;");
        ui.PasswordReveal->setText("On");
    }
}

void AutoLaunchWizard101C::revealText(QPushButton* button, int index) {
    QJsonArray settings = jsonData["settings"].toArray();
    while (settings.size() < 2) settings.append("Off");

    if (index == 0) {
        bool visible = ui.inputUsername->echoMode() == QLineEdit::Normal;
        ui.inputUsername->setEchoMode(visible ? QLineEdit::Password : QLineEdit::Normal);
        settings[0] = visible ? "Off" : "On";
    }
    else if (index == 1) {
        bool visible = ui.inputPassword->echoMode() == QLineEdit::Normal;
        ui.inputPassword->setEchoMode(visible ? QLineEdit::Password : QLineEdit::Normal);
        settings[1] = visible ? "Off" : "On";
    }

    if (button->text() == "On") {
        button->setStyleSheet("background-color: red; color: white;");
        button->setText("Off");
    }
    else {
        button->setStyleSheet("background-color: green; color: white;");
        button->setText("On");
    }

    jsonData["settings"] = settings;
    saveJson();
}