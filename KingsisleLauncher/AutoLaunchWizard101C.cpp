// Yes I know this is bad coding gonna seperate this into seperate classes eventually
#include "AutoLaunchWizard101C.h"
#include "ui_AutoLaunchWizard101C.h"
#include <windows.h>
#include <QtConcurrent/qtconcurrentrun.h>
#include <TlHelp32.h>

AutoLaunchWizard101C::AutoLaunchWizard101C(QWidget* parent)
{
    ui.setupUi(this);
    setWindowTitle("Ju3dge");
    loadAccountsFromFile();
    loadPathsFromFile();
    loadBundlesFromFile();
    ui.GameDropbox->addItem("Wizard101");
    ui.GameDropbox->addItem("Pirate101");
    ui.inputPassword->setEchoMode(QLineEdit::Password);
    connect(ui.AddAccountButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::addAccount);
    connect(ui.DeleteAccountButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::deleteAccount);
    connect(ui.AddBundleButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::addBundleAccount);
    connect(ui.DeleteBundleButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::deleteBundleAccount);
    connect(ui.BrowseButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::browse);
    connect(ui.GameDropbox, &QComboBox::currentTextChanged, this, &AutoLaunchWizard101C::gameSelect);
    connect(ui.LaunchButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::launch);
    connect(ui.BundleLaunchButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::bundleLaunch);
    connect(ui.killAllButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::killAllClients);
    connect(ui.SpoofButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::spoof);
    
    // Display 
    connect(ui.NicknameDropbox, &QComboBox::currentIndexChanged, this, &AutoLaunchWizard101C::displayTopText);
    connect(ui.BundleNicknameDropbox, &QComboBox::currentIndexChanged, this, &AutoLaunchWizard101C::displayMiddleText);

    connect(ui.SaveUserButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::saveUser);
    connect(ui.SaveBundleButton, &QPushButton::clicked, this, &AutoLaunchWizard101C::saveBundle);
}



void AutoLaunchWizard101C::saveBundle() {
    QString currentBundleNickname = ui.BundleNicknameDropbox->currentText();
    QString bundleNickname = ui.inputBundleNickname->text().trimmed();
    QString massNicknames = ui.inputMassNicknames->text().trimmed();

    if (bundleNickname.isEmpty() || massNicknames.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Both fields must be filled out before saving");
        return;
    }

    // Check if current bundle exists
    if (!bundleAccounts.contains(currentBundleNickname)) {
        QMessageBox::warning(this, "Save Failed", "The selected bundle could not be found.");
        return;
    }

    // Update in-memory map
    QStringList massNickList = massNicknames.split('/', Qt::SkipEmptyParts);
    bundleAccounts.remove(currentBundleNickname);         // Remove old key
    bundleAccounts[bundleNickname] = massNickList;        // Add new/updated one

    // Update bundles.txt
    QFile file("information/bundles.txt");
    QStringList lines;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "File Error", "Failed to open bundles.txt for reading.");
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split('=');
        if (parts.size() == 2 && parts[0] == currentBundleNickname) {
            // Replace this line with updated nickname and nicknames
            line = bundleNickname + "=" + massNickList.join("/");
        }
        lines.append(line);
    }
    file.close();

    // Write updated lines back to the file
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, "File Error", "Failed to open bundles.txt for writing.");
        return;
    }

    QTextStream out(&file);
    for (const QString& line : lines) {
        out << line << "\n";
    }
    file.close();

    // Update the dropdown
    int index = ui.BundleNicknameDropbox->findText(currentBundleNickname);
    if (index != -1) {
        ui.BundleNicknameDropbox->setItemText(index, bundleNickname);
        ui.BundleNicknameDropbox->setCurrentIndex(index);
    }

    QMessageBox::information(this, "Success", "Bundle info updated successfully.");
}

void AutoLaunchWizard101C::saveUser() {
    QString currentAccountNickname = ui.NicknameDropbox->currentText();
    QString nickname = ui.inputNickname->text().trimmed();
    QString username = ui.inputUsername->text().trimmed();
    QString password = ui.inputPassword->text().trimmed();

    if (nickname.isEmpty() || username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields must be filled out before saving");
        return;
    }

    // Find and update the matching account
    bool found = false;
    for (AccountInfo& account : accounts) {
        if (account.nickname == currentAccountNickname) {
            account.nickname = nickname;
            account.username = username;
            account.password = password;

            QFile file("information/info.txt");
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                for (const AccountInfo& acc : accounts) {
                    out << acc.nickname << "/" << acc.username << "/" << acc.password << "\n";
                }
                file.close();
            }
            found = true;
            break;
        }
    }
    if (!found) {
        QMessageBox::warning(this, "Save Failed", "The selected account could not be found in the list.");
        return;
    }

    // update info.txt 
    QStringList lines;
    QFile file("information/info.txt");
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split('/');
        if (parts.size() >= 3 && parts[0] == currentAccountNickname) {
            line = nickname + "/" + username + "/" + password;  // Replace the line with updated values
        }
        lines.append(line);
    }
    file.close();

    // update dropbox for new nickname if changed
    int index = ui.NicknameDropbox->findText(currentAccountNickname);
    if (index != -1) {
        ui.NicknameDropbox->setItemText(index, nickname);
        ui.NicknameDropbox->setCurrentIndex(index); // Optionally reselect it
    }

    QMessageBox::information(this, "Success", "Account info updated successfully.");
}


void AutoLaunchWizard101C::displayMiddleText() {
    QString nickname = ui.BundleNicknameDropbox->currentText();

    if (bundleAccounts.contains(nickname)) {
        ui.inputBundleNickname->setText(nickname);
        ui.inputMassNicknames->setText(bundleAccounts[nickname].join("/"));

    }
    else {
        // Bundle not found
        ui.inputBundleNickname->clear();
        ui.inputMassNicknames->clear();
        QMessageBox::warning(this, "Bundle Not Found", QString("No bundle found with nickname: %1").arg(nickname));
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
        QMessageBox::warning(this, "Account Not Found", QString("No account found with nickname: %1").arg(nickname));
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

            std::wstring windowTitle = selectedAccount.nickname.toStdWString() + game.toStdWString();
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

void AutoLaunchWizard101C::savePathsToFile() {
    QFile file("information/path.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << wizardPath.trimmed() << "\n";
        out << piratePath.trimmed() << "\n";
        file.close();
    }
}

void AutoLaunchWizard101C::loadAccountsFromFile() {
    QFile file("information/info.txt");
    if (!file.exists()) {
        QFile newFile("information/info.txt");
        newFile.open(QIODevice::WriteOnly);
        newFile.close();
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split('/');
        if (parts.size() == 3) {
            AccountInfo acc{ parts[0], parts[1], parts[2] };
            accounts.append(acc);
            ui.NicknameDropbox->addItem(acc.nickname);
        }
    }
    file.close();
}

void AutoLaunchWizard101C::loadPathsFromFile() {
    QFile file("information/path.txt");
    if (!file.exists()) {
        QFile newFile("information/path.txt");
        if (newFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&newFile);
            out << "C:/ProgramData/KingsIsle Entertainment/Wizard101/Bin\n";
            out << "C:/ProgramData/KingsIsle Entertainment/Pirate101/Bin\n";
            newFile.close();
        }
    }
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString wizard = in.readLine().trimmed();
        QString pirate = in.readLine().trimmed();
        ui.inputWizardPath->setText(wizard.isEmpty() ? "C:/..." : wizard);
        wizardPath = wizard;
        piratePath = pirate;
        file.close();
    }
}

void AutoLaunchWizard101C::loadBundlesFromFile() {
    QFile file("information/bundles.txt");
    if (!file.exists()) return;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;

            QStringList parts = line.split('=');
            if (parts.size() != 2) continue;

            QString bundleName = parts[0].trimmed();
            QStringList names = parts[1].split("/", Qt::SkipEmptyParts);

            if (!bundleAccounts.contains(bundleName)) {
                bundleAccounts[bundleName] = names;
                ui.BundleNicknameDropbox->addItem(bundleName);
            }
        }
        file.close();
    }
}

AutoLaunchWizard101C::~AutoLaunchWizard101C(){}

void AutoLaunchWizard101C::addAccount()
{
    QString nickname = ui.inputNickname->text().trimmed();
    QString username = ui.inputUsername->text().trimmed();
    QString password = ui.inputPassword->text().trimmed();

    if (nickname.isEmpty() || username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields must be filled out.");
        return;
    }

    for (const AccountInfo& acc : accounts) {
        if (acc.nickname == nickname) {
            QMessageBox::warning(this, "Duplicate", "Nickname already exists.");
            return;
        }
    }

    AccountInfo newAccount{ nickname, username, password };
    accounts.append(newAccount);
    ui.NicknameDropbox->addItem(nickname);
    

    QFile file("information/info.txt");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << nickname << "/" << username << "/" << password << "\n";
        file.close();
    }

    ui.inputNickname->clear();
    ui.inputUsername->clear();
    ui.inputPassword->clear();
}

void AutoLaunchWizard101C::deleteAccount()
{
    int index = ui.NicknameDropbox->currentIndex();
    if (index < 0 || index >= accounts.size()) {
        QMessageBox::warning(this, "Delete Error", "No account selected.");
        return;
    }

    QString removedName = accounts[index].nickname;
    accounts.removeAt(index);
    ui.NicknameDropbox->removeItem(index);

    QFile file("information/info.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const AccountInfo& acc : accounts) {
            out << acc.nickname << "/" << acc.username << "/" << acc.password << "\n";
        }
        file.close();
    }

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

void AutoLaunchWizard101C::saveBundlesToFile() {
    QFile file("information/bundles.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (auto it = bundleAccounts.begin(); it != bundleAccounts.end(); ++it) {
            out << it.key() << "=" << it.value().join("/") << "\n";
        }
        file.close();
    }
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
