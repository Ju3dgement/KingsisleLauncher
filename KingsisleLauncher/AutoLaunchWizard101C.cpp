#include "AutoLaunchWizard101C.h"
#include "ui_AutoLaunchWizard101C.h"
#include "topSection.h"
#include "middleSection.h"
#include "bottomSection.h"


AccountManager* accountManager;
BundleManager* bundleManager;
MiscManager* miscManager;

AutoLaunchWizard101C::AutoLaunchWizard101C(QWidget* parent)
{
    ui.setupUi(this);
    setWindowTitle("Ju3dge Launcher");
    setWindowIcon(QIcon("images/Ju3dge.ico"));

    // UI stuff I couldn't find inside of the ui file
    ui.GameDropbox->addItem("Wizard101");
    ui.GameDropbox->addItem("Pirate101");
    ui.SaveUserButton->hide();
    ui.SaveBundleButton->hide();
    ui.inputWizardPath->setReadOnly(true);
    ui.SaveUserButton->setStyleSheet("background-color: green; color: white;");
    ui.SaveBundleButton->setStyleSheet("background-color: green; color: white;");

    // Loads data from json
    loadAccountsFromFile();
    loadPathsFromFile();
    loadBundlesFromFile();
    loadSettings();

    // Top Section
    accountManager = new AccountManager(&ui, &accounts, &jsonData, this);
    connect(ui.AddAccountButton, &QPushButton::clicked, accountManager, &AccountManager::addAccount);
    connect(ui.DeleteAccountButton, &QPushButton::clicked, accountManager, &AccountManager::deleteAccount);
    connect(ui.SaveUserButton, &QPushButton::clicked, accountManager, &AccountManager::saveUser);
    connect(ui.NicknameDropbox, &QComboBox::activated, accountManager, &AccountManager::displayTopText);
    connect(ui.inputNickname, &QLineEdit::textChanged, accountManager, &AccountManager::changeText);
    connect(ui.inputUsername, &QLineEdit::textChanged, accountManager, &AccountManager::changeText);
    connect(ui.inputPassword, &QLineEdit::textChanged, accountManager, &AccountManager::changeText);
    connect(ui.UsernameReveal, &QPushButton::clicked, accountManager, [=]() {accountManager->revealText(ui.UsernameReveal, 0);});
    connect(ui.PasswordReveal, &QPushButton::clicked, accountManager, [=]() {accountManager->revealText(ui.PasswordReveal, 1);});
    connect(ui.LaunchButton, &QPushButton::clicked, accountManager, &AccountManager::launch);

	// Middle Section
	bundleManager = new BundleManager(&ui, &bundleAccounts, &jsonData, &accounts, this);
    connect(ui.AddBundleButton, &QPushButton::clicked, bundleManager, &BundleManager::addBundleAccount);
    connect(ui.DeleteBundleButton, &QPushButton::clicked, bundleManager, &BundleManager::deleteBundleAccount);
    connect(ui.SaveBundleButton, &QPushButton::clicked, bundleManager, &BundleManager::saveBundle);
    connect(ui.BundleNicknameDropbox, &QComboBox::activated, bundleManager, &BundleManager::displayMiddleText);
    connect(ui.inputNickname, &QLineEdit::textChanged, bundleManager, &BundleManager::changeText);
    connect(ui.inputMassNicknames, &QLineEdit::textChanged, bundleManager, &BundleManager::changeText);
    connect(ui.BundleLaunchButton, &QPushButton::clicked, bundleManager, &BundleManager::bundleLaunch);

    // Bottom Section
	miscManager = new MiscManager(&ui, &jsonData, &wizardPath, &piratePath, this);
    connect(ui.BrowseButton, &QPushButton::clicked, miscManager, &MiscManager::browse);
    connect(ui.killAllButton, &QPushButton::clicked, miscManager, &MiscManager::killAllClients);
    connect(ui.SpoofButton, &QPushButton::clicked, miscManager, &MiscManager::spoof);
    connect(ui.GameDropbox, &QComboBox::activated, miscManager, &MiscManager::gameSelect);
    connect(ui.injectDLLButton, &QPushButton::clicked, miscManager, &MiscManager::prepareInjectDLL);
    
}


AutoLaunchWizard101C::~AutoLaunchWizard101C() {}

void AutoLaunchWizard101C::showStyledWarning(QWidget* parent, const QString& title, const QString& text, bool warningIcon) {
    QMessageBox msgBox(parent);
    if (warningIcon) {
        msgBox.setIcon(QMessageBox::Warning); 
    } else {
        msgBox.setIcon(QMessageBox::Information); 
	}
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setStyleSheet(
        "QLabel { color: white; }"
        "QPushButton { background-color: white; color: black; }"
    );
    msgBox.exec();
}

void AutoLaunchWizard101C::launchAccount(AccountInfo& selectedAccount, QString& game) {
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
        for (int i = 0; i < 50 && hwnd == nullptr; ++i) {
            hwnd = FindWindowW(nullptr, (LPCWSTR)game.utf16());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }


        if (hwnd) {
            for (QChar character : selectedAccount.username)
                SendMessageW(hwnd, WM_CHAR, character.unicode(), 0); // username

            SendMessageW(hwnd, WM_CHAR, 9, 0); // Tab

            for (QChar character : selectedAccount.password)
                SendMessageW(hwnd, WM_CHAR, character.unicode(), 0); // password

            SendMessageW(hwnd, WM_CHAR, 13, 0); // Enter

            std::wstring windowTitle = selectedAccount.nickname.toStdWString();
            SetWindowTextW(hwnd, windowTitle.c_str());

            // dll
            if (!DLLPath.isEmpty()) {
                miscManager->injectDLLToProcess(pi.dwProcessId, DLLPath);
            }
        }

        else {
            QMetaObject::invokeMethod(this, [=]() {
                QMessageBox::warning(this, "Window Not Found", QString("Could not find game window %1 after launch.").arg(selectedAccount.nickname));
                }, Qt::QueuedConnection);
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        });

}


void AutoLaunchWizard101C::saveJson() {
    QFile file("information/data.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(QJsonDocument(jsonData).toJson());
        file.close();
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

void AutoLaunchWizard101C::loadBundlesFromFile() {
    ui.BundleNicknameDropbox->clear();
    bundleAccounts.clear();
    QJsonObject bundles = jsonData["bundles"].toObject();
    for ( QString& key : bundles.keys()) {
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