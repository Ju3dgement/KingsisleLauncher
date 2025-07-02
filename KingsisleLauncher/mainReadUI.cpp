#include "AutoLaunchWizard101C.h"
#include <QtWidgets/QApplication>

//int main(int argc, char *argv[])
//{
//    QApplication app(argc, argv);
//    AutoLaunchWizard101C window;
//    window.show();
//    return app.exec();
//}

#include <QApplication>
#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFrame>
#include <QMap>
#include <algorithm>

struct AccountInfo {
    QString nickname;
    QString username;
    QString password;
};

class AutoLoginWindow : public QMainWindow {
    Q_OBJECT

public:
    AutoLoginWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        setWindowTitle("Ju3dge's AutoLauncher");

        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
        QHBoxLayout* topLayout = new QHBoxLayout;
        QGridLayout* formLayout = new QGridLayout;
        QHBoxLayout* buttonLayout = new QHBoxLayout;
        QHBoxLayout* launchLayout = new QHBoxLayout;

        accountCombo = new QComboBox;
        topLayout->addWidget(accountCombo);

        nicknameEdit = new QLineEdit;
        usernameEdit = new QLineEdit;
        passwordEdit = new QLineEdit;
        passwordEdit->setEchoMode(QLineEdit::Password);

        formLayout->addWidget(new QLabel("Nickname:"), 0, 0);
        formLayout->addWidget(nicknameEdit, 0, 1);
        formLayout->addWidget(new QLabel("Username:"), 1, 0);
        formLayout->addWidget(usernameEdit, 1, 1);
        formLayout->addWidget(new QLabel("Password:"), 2, 0);
        formLayout->addWidget(passwordEdit, 2, 1);

        topLayout->addLayout(formLayout);
        mainLayout->addLayout(topLayout);

        QPushButton* addAccountButton = new QPushButton("Add Account");
        QPushButton* deleteAccountButton = new QPushButton("Delete Account");
        buttonLayout->addWidget(addAccountButton);
        buttonLayout->addWidget(deleteAccountButton);
        mainLayout->addLayout(buttonLayout);

        // Separator line
        QFrame* line = new QFrame;
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        mainLayout->addWidget(line);

        // Bundle account area
        QGridLayout* bundleLayout = new QGridLayout;
        bundleCombo = new QComboBox;
        bundleNicknameEdit = new QLineEdit;
        bundleMassNicknamesEdit = new QLineEdit;
        QPushButton* addBundleButton = new QPushButton("Add Bundle Account");
        QPushButton* deleteBundleButton = new QPushButton("Delete Bundle");

        bundleLayout->addWidget(new QLabel("Bundle:"), 0, 0);
        bundleLayout->addWidget(bundleCombo, 0, 1);
        bundleLayout->addWidget(deleteBundleButton, 0, 2);
        bundleLayout->addWidget(new QLabel("Bundle Nickname:"), 1, 0);
        bundleLayout->addWidget(bundleNicknameEdit, 1, 1);
        bundleLayout->addWidget(new QLabel("Mass Nicknames (e.g. Joe/Bob/Harley):"), 2, 0);
        bundleLayout->addWidget(bundleMassNicknamesEdit, 2, 1);
        bundleLayout->addWidget(addBundleButton, 2, 2);
        mainLayout->addLayout(bundleLayout);

        // Wizard path
        wizardPathWidget = new QWidget;
        QHBoxLayout* wizardPathLayout = new QHBoxLayout(wizardPathWidget);
        wizardPathEdit = new QLineEdit;
        QPushButton* browseWizard = new QPushButton("Browse");
        wizardPathLayout->addWidget(new QLabel("Wizard101 Path:"));
        wizardPathLayout->addWidget(wizardPathEdit);
        wizardPathLayout->addWidget(browseWizard);
        wizardPathWidget->setLayout(wizardPathLayout);
        mainLayout->addWidget(wizardPathWidget);

        // Pirate path
        piratePathWidget = new QWidget;
        QHBoxLayout* piratePathLayout = new QHBoxLayout(piratePathWidget);
        piratePathEdit = new QLineEdit;
        QPushButton* browsePirate = new QPushButton("Browse");
        piratePathLayout->addWidget(new QLabel("Pirate101 Path:"));
        piratePathLayout->addWidget(piratePathEdit);
        piratePathLayout->addWidget(browsePirate);
        piratePathWidget->setLayout(piratePathLayout);
        mainLayout->addWidget(piratePathWidget);

        // Game selection and launch
        gameSelect = new QComboBox;
        gameSelect->addItem("Wizard101");
        gameSelect->addItem("Pirate101");

        QPushButton* launchButton = new QPushButton("Launch");
        bundleLaunchButton = new QPushButton("Bundle Launch");

        launchLayout->addStretch();
        launchLayout->addWidget(new QLabel("Game:"));
        launchLayout->addWidget(gameSelect);
        launchLayout->addWidget(launchButton);
        launchLayout->addWidget(bundleLaunchButton);
        mainLayout->addLayout(launchLayout);

        logOutput = new QTextEdit;
        logOutput->setReadOnly(true);
        mainLayout->addWidget(logOutput);

        loadAccountsFromFile();
        loadPathsFromFile();
        loadBundlesFromFile();

        connect(accountCombo, &QComboBox::currentIndexChanged, this, &AutoLoginWindow::onAccountSelected);
        connect(browseWizard, &QPushButton::clicked, this, &AutoLoginWindow::browseWizardPath);
        connect(browsePirate, &QPushButton::clicked, this, &AutoLoginWindow::browsePiratePath);
        connect(launchButton, &QPushButton::clicked, this, &AutoLoginWindow::launchGame);
        connect(addAccountButton, &QPushButton::clicked, this, &AutoLoginWindow::addAccount);
        connect(deleteAccountButton, &QPushButton::clicked, this, &AutoLoginWindow::deleteAccount);
        connect(gameSelect, &QComboBox::currentTextChanged, this, &AutoLoginWindow::updatePathVisibility);
        connect(addBundleButton, &QPushButton::clicked, this, &AutoLoginWindow::addBundleAccount);
        connect(deleteBundleButton, &QPushButton::clicked, this, &AutoLoginWindow::deleteBundleAccount);
        connect(bundleLaunchButton, &QPushButton::clicked, this, &AutoLoginWindow::launchBundle);

        updatePathVisibility(gameSelect->currentText());
    }

private slots:
    void onAccountSelected(int index) {
        if (index >= 0 && index < accounts.size()) {
            const AccountInfo& acc = accounts[index];
            nicknameEdit->setText(acc.nickname);
            usernameEdit->setText(acc.username);
            passwordEdit->setText(acc.password);
        }
    }

    void browseWizardPath() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Wizard101 Path");
        if (!dir.isEmpty()) {
            wizardPathEdit->setText(dir);
            savePathsToFile();
        }
    }

    void browsePiratePath() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Pirate101 Path");
        if (!dir.isEmpty()) {
            piratePathEdit->setText(dir);
            savePathsToFile();
        }
    }

    void launchGame() {
        QString game = gameSelect->currentText();
        QString account = accountCombo->currentText();
        QString path = (game == "Wizard101") ? wizardPathEdit->text() : piratePathEdit->text();
        logOutput->append("Launching " + game + " from: " + path + " as " + account);
    }

    void launchBundle() {
        QString bundle = bundleCombo->currentText();
        if (!bundleAccounts.contains(bundle)) {
            QMessageBox::warning(this, "Error", "No such bundle.");
            return;
        }

        QStringList nicknames = bundleAccounts[bundle];
        QString game = gameSelect->currentText();
        QString path = (game == "Wizard101") ? wizardPathEdit->text() : piratePathEdit->text();

        logOutput->append("Launching bundle: " + bundle);
        for (const QString& name : nicknames) {
            logOutput->append("Launching " + game + " from: " + path + " as " + name);
        }
    }

    void addAccount() {
        QString nickname = nicknameEdit->text().trimmed();
        QString username = usernameEdit->text().trimmed();
        QString password = passwordEdit->text().trimmed();

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
        accountCombo->addItem(nickname);

        QFile file("info.txt");
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out << nickname << "/" << username << "/" << password << "\n";
            file.close();
            logOutput->append("Account added: " + nickname);
        }
    }

    void deleteAccount() {
        int index = accountCombo->currentIndex();
        if (index < 0 || index >= accounts.size()) {
            QMessageBox::warning(this, "Delete Error", "No account selected.");
            return;
        }

        QString removedName = accounts[index].nickname;
        accounts.removeAt(index);
        accountCombo->removeItem(index);

        QFile file("info.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (const AccountInfo& acc : accounts) {
                out << acc.nickname << "/" << acc.username << "/" << acc.password << "\n";
            }
            file.close();
            logOutput->append("Deleted account: " + removedName);
        }

        if (!accounts.isEmpty()) {
            accountCombo->setCurrentIndex(0);
            onAccountSelected(0);
        }
        else {
            nicknameEdit->clear();
            usernameEdit->clear();
            passwordEdit->clear();
        }
    }

    void updatePathVisibility(const QString& game) {
        bool isWizard = (game == "Wizard101");
        wizardPathWidget->setVisible(isWizard);
        piratePathWidget->setVisible(!isWizard);
    }

    void addBundleAccount() {
        QString nickname = bundleNicknameEdit->text().trimmed();
        QString massNick = bundleMassNicknamesEdit->text().trimmed();

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
        bundleCombo->addItem(nickname);
        logOutput->append("Bundle added: " + nickname);
        saveBundlesToFile();
    }

    void deleteBundleAccount() {
        QString bundle = bundleCombo->currentText();
        if (!bundleAccounts.contains(bundle)) {
            QMessageBox::warning(this, "Error", "No such bundle.");
            return;
        }

        bundleAccounts.remove(bundle);
        int index = bundleCombo->findText(bundle);
        if (index >= 0)
            bundleCombo->removeItem(index);

        logOutput->append("Bundle deleted: " + bundle);
        saveBundlesToFile();
    }

private:
    void loadAccountsFromFile() {
        QFile file("info.txt");
        if (!file.exists()) {
            QFile newFile("info.txt");
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
                accountCombo->addItem(acc.nickname);
            }
        }
        file.close();

        if (!accounts.isEmpty()) {
            accountCombo->setCurrentIndex(0);
            onAccountSelected(0);
        }
    }

    void loadPathsFromFile() {
        QFile file("path.txt");
        if (!file.exists()) {
            QFile newFile("path.txt");
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
            wizardPathEdit->setText(wizard.isEmpty() ? "C:/..." : wizard);
            piratePathEdit->setText(pirate.isEmpty() ? "C:/..." : pirate);
            file.close();
        }
    }

    void savePathsToFile() {
        QFile file("path.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << wizardPathEdit->text().trimmed() << "\n";
            out << piratePathEdit->text().trimmed() << "\n";
            file.close();
        }
    }

    void loadBundlesFromFile() {
        QFile file("bundles.txt");
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
                    bundleCombo->addItem(bundleName);
                }
            }
            file.close();
        }
    }

    void saveBundlesToFile() {
        QFile file("bundles.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (auto it = bundleAccounts.begin(); it != bundleAccounts.end(); ++it) {
                out << it.key() << "=" << it.value().join("/") << "\n";
            }
            file.close();
        }
    }


    // Helper to reload bundles.txt and update the map and combo box silently (no UI change)
    void reloadBundlesFromFile() {
        QFile file("bundles.txt");
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

    void onBundleSelected(const QString& bundleName) {
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

    QComboBox* accountCombo;
    QLineEdit* nicknameEdit;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QLineEdit* wizardPathEdit;
    QLineEdit* piratePathEdit;
    QComboBox* gameSelect;
    QTextEdit* logOutput;
    QWidget* wizardPathWidget;
    QWidget* piratePathWidget;

    QComboBox* bundleCombo;
    QLineEdit* bundleNicknameEdit;
    QLineEdit* bundleMassNicknamesEdit;
    QPushButton* bundleLaunchButton;

    QList<AccountInfo> accounts;
    QMap<QString, QStringList> bundleAccounts;
};













