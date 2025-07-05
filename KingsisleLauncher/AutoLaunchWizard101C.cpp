#include "AutoLaunchWizard101C.h"
#include "ui_AutoLaunchWizard101C.h"
#include "AccountManager.h"
#include "BundleManager.h"

AutoLaunchWizard101C::AutoLaunchWizard101C(QWidget* parent)
{
    ui.setupUi(this);
    setWindowTitle("Ju3dge Launcher");
    setWindowIcon(QIcon("images/Ju3dge.ico"));

    ui.GameDropbox->addItem("Wizard101");
    ui.GameDropbox->addItem("Pirate101");
    ui.SaveUserButton->hide();
    ui.SaveBundleButton->hide();
    loadAccountsFromFile();
    loadPathsFromFile();
    loadBundlesFromFile();
    loadSettings();
    ui.inputWizardPath->setReadOnly(true);
    ui.SaveUserButton->setStyleSheet("background-color: green; color: white;");
    ui.SaveBundleButton->setStyleSheet("background-color: green; color: white;");

    // Top section
    AccountManager* accountManager = new AccountManager(&ui, &accounts, &jsonData, this);
    connect(ui.AddAccountButton, &QPushButton::clicked, accountManager, &AccountManager::addAccount);
    connect(ui.DeleteAccountButton, &QPushButton::clicked, accountManager, &AccountManager::deleteAccount);
    connect(ui.SaveUserButton, &QPushButton::clicked, accountManager, &AccountManager::saveUser);
    connect(ui.NicknameDropbox, &QComboBox::activated, accountManager, &AccountManager::displayTopText);

	// Middle section
	BundleManager* bundleManager = new BundleManager(&ui, &bundleAccounts, &jsonData, &accounts, this);
    connect(ui.AddBundleButton, &QPushButton::clicked, bundleManager, &BundleManager::addBundleAccount);
    connect(ui.DeleteBundleButton, &QPushButton::clicked, bundleManager, &BundleManager::deleteBundleAccount);
    connect(ui.SaveBundleButton, &QPushButton::clicked, bundleManager, &BundleManager::saveBundle);
    connect(ui.BundleNicknameDropbox, &QComboBox::activated, bundleManager, &BundleManager::displayMiddleText);



    connect(ui.BrowseButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::browse);
    connect(ui.LaunchButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::launch);
    connect(ui.BundleLaunchButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::bundleLaunch);
    connect(ui.killAllButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::killAllClients);    
    connect(ui.SpoofButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::spoof);
    connect(ui.GameDropbox, &QComboBox::activated, this, &AutoLaunchWizard101C::gameSelect);
    connect(ui.UsernameReveal, &QPushButton::clicked, this, [=]() {revealText(ui.UsernameReveal, 0);});
    connect(ui.PasswordReveal, &QPushButton::clicked, this, [=]() {revealText(ui.PasswordReveal, 1);});
    connect(ui.inputNickname, &QLineEdit::textChanged, this, [=]() {changedText(0);});
    connect(ui.inputUsername, &QLineEdit::textChanged, this, [=]() {changedText(0);});
    connect(ui.inputPassword, &QLineEdit::textChanged, this, [=]() {changedText(0);});
    connect(ui.inputNickname, &QLineEdit::textChanged, this, [=]() {changedText(1);});
    connect(ui.inputMassNicknames, &QLineEdit::textChanged, this, [=]() {changedText(1);});
}

void AutoLaunchWizard101C::changedText(int index) {
    if (index == 0) {
        QString nickname = ui.inputNickname->text().trimmed();
        QString username = ui.inputUsername->text().trimmed();
        QString password = ui.inputPassword->text().trimmed();

        loadJson();
        QJsonArray accs = jsonData["accounts"].toArray();
        bool matchFound = false;

        for (const QJsonValue& val : accs) {
            QJsonObject obj = val.toObject();
            if (obj["nickname"].toString() == nickname) {
                matchFound = true;
                QString storedUsername = obj["username"].toString();
                QString storedPassword = obj["password"].toString();

                if (storedUsername != username || storedPassword != password) {
                    ui.SaveUserButton->show();
                }
                else {
                    ui.SaveUserButton->hide();
                }
                break;
            }
        }

        if (!matchFound) {
            ui.SaveUserButton->show();
        }
    }
    else if (index == 1) {
        QString bundleNickname = ui.inputBundleNickname->text().trimmed();
        QString massNicknames = ui.inputMassNicknames->text().trimmed();

        loadJson();
        QJsonObject bundlesObj = jsonData["bundles"].toObject();
        bool matchFound = false;

        if (bundlesObj.contains(bundleNickname)) {
            matchFound = true;
            QJsonArray storedArray = bundlesObj[bundleNickname].toArray();
            QStringList storedList;
            for (const QJsonValue& val : storedArray)
                storedList.append(val.toString());

            QString storedMassNicknames = storedList.join("/");

            if (storedMassNicknames != massNicknames) {
                ui.SaveBundleButton->show();
            }
            else {
                ui.SaveBundleButton->hide();
            }
        }

        if (!matchFound) {
            ui.SaveBundleButton->show();
        }
    }
}

AutoLaunchWizard101C::~AutoLaunchWizard101C() {}

void AutoLaunchWizard101C::showStyledWarning(QWidget* parent, const QString& title, const QString& text, bool warningIcon) {
    QMessageBox msgBox(parent);
    if (warningIcon) {
        msgBox.setIcon(QMessageBox::Warning);  // warning icon
    } else {
        msgBox.setIcon(QMessageBox::Information);  // info icon
	}
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setStyleSheet("QLabel { color: white; }"); 
    msgBox.exec();
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