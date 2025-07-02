// Yes I know this is bad coding format deal with it
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
    QString exePath = (game == "Wizard101") ? wizardPath : piratePath;
    QString exeName = (game == "Wizard101") ? "WizardGraphicalClient.exe" : "Pirate.exe";
    QString fullPath = exePath + "/Bin/" + exeName;
    QString nickname = selectedAccount.nickname;

    QtConcurrent::run([=]() {
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        QString cmd = QString("\"%1\" -L login.us.%2.com 12000").arg(fullPath, game.toLower());

        if (!CreateProcessW(nullptr,
            (LPWSTR)cmd.utf16(),
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            (LPCWSTR)(exePath + "/Bin").utf16(),
            &si,
            &pi)) {
            QMetaObject::invokeMethod(this, [=]() {
                QMessageBox::warning(this, "Launch Failed", QString("Could not launch client for %1.").arg(nickname));
                }, Qt::QueuedConnection);
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        HWND hwnd = nullptr;
        for (int i = 0; i < 50 && hwnd == nullptr; ++i) {
            hwnd = FindWindowW(nullptr, (LPCWSTR)game.utf16());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (hwnd) {
            for (QChar c : selectedAccount.username) SendMessageW(hwnd, WM_CHAR, c.unicode(), 0);
            SendMessageW(hwnd, WM_CHAR, 9, 0);
            for (QChar c : selectedAccount.password) SendMessageW(hwnd, WM_CHAR, c.unicode(), 0);
            SendMessageW(hwnd, WM_CHAR, 13, 0);

            std::wstring windowTitle = nickname.toStdWString() + game.toStdWString();
            SetWindowTextW(hwnd, windowTitle.c_str());
        }
        });
}

void AutoLaunchWizard101C::launch() {
    QString nickname = ui.NicknameDropbox->currentText();
    if (nickname.isEmpty()) return;

    auto it = std::find_if(accounts.begin(), accounts.end(), [&](const AccountInfo& a) {
        return a.nickname == nickname;
        });

    if (it == accounts.end()) return;

    QString game = ui.GameDropbox->currentText();
    launchAccount(*it, game);
}

void AutoLaunchWizard101C::bundleLaunch() {
    QString bundleName = ui.BundleNicknameDropbox->currentText();
    if (bundleName.isEmpty() || !bundleAccounts.contains(bundleName)) return;

    QString game = ui.GameDropbox->currentText();  // use current dropdown selection

    const QStringList& nicknames = bundleAccounts[bundleName];

    for (const QString& nick : nicknames) {
        auto it = std::find_if(accounts.begin(), accounts.end(), [&](const AccountInfo& a) {
            return a.nickname == nick;
            });
        if (it != accounts.end()) {
            launchAccount(*it, game); // launch each in its own thread
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // optional: stagger launches
        }
    }
}

void AutoLaunchWizard101C::browse(){
    if (ui.GameDropbox->currentText() == "Wizard101") {
        browseWizardPath();
    } else if (ui.GameDropbox->currentText() == "Pirate101") {
        browsePiratePath();
    } else {
		QMessageBox::warning(this, "Error", "How did you get this error?");
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


void AutoLaunchWizard101C::onBundleSelected(const QString& bundleName) {
    if (bundleName.isEmpty()) {
        bundleMassNicknamesEdit->clear();
        bundleNicknameEdit->clear();
        return;
    }

    // Reload bundles from file to get freshest data
    reloadBundlesFromFile();

    if (bundleAccounts.contains(bundleName)) {
        bundleMassNicknamesEdit->setText(bundleAccounts[bundleName].join("/"));
        bundleNicknameEdit->setText(bundleName);
    }
    else {
        bundleMassNicknamesEdit->clear();
        bundleNicknameEdit->clear();
    }
}

void AutoLaunchWizard101C::reloadBundlesFromFile() {
    QFile file("information/bundles.txt");
    QMap<QString, QStringList> newBundles;
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

            newBundles[bundleName] = names;
        }
        file.close();
    }

    bundleAccounts = newBundles;

    // Update the combo box items silently:
    QString current = bundleCombo->currentText();
    bundleCombo->blockSignals(true);
    bundleCombo->clear();
    for (auto it = bundleAccounts.begin(); it != bundleAccounts.end(); ++it) {
        bundleCombo->addItem(it.key());
    }
    int idx = bundleCombo->findText(current);
    if (idx >= 0)
        bundleCombo->setCurrentIndex(idx);
    bundleCombo->blockSignals(false);
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

    //if (!accounts.isEmpty()) {
    //    accountCombo->setCurrentIndex(0);
    //    onAccountSelected(0);
    //}
}

void AutoLaunchWizard101C::loadPathsFromFile() {
    QFile file("information/path.txt");
    if (!file.exists()) {
        QFile newFile("information/path.txt");
        if (newFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&newFile);
            out << "C:/ProgramData/KingsIsle Entertainment/Wizard101/\n";
            out << "C:/ProgramData/KingsIsle Entertainment/Pirate101/\n";
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
        //ui.Wizard101PathLabel->setText(pirate.isEmpty() ? "C:/..." : pirate);
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
                //bundleCombo->addItem(bundleName);
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
        //ui->logOutput->append("Account added: " + nickname);
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
        //ui->logOutput->append("Deleted account: " + removedName);
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
    //logOutput->append("Bundle added: " + nickname);
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

    //logOutput->append("Bundle deleted: " + bundle);
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
