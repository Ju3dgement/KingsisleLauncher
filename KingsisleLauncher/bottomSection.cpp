#include "bottomSection.h"

MiscManager::MiscManager(Ui::AutoLaunchWizard101CClass* uiPtr,  QJsonObject* jsonPtr, QString* wizardPath, QString* piratePath, AutoLaunchWizard101C* parent)
    : ui(uiPtr),  jsonData(jsonPtr), wizardPath(wizardPath), piratePath(piratePath), parent(parent) {
}

void MiscManager::killAllClients() {
    QString targetName = "WizardGraphicalClient.exe";

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        parent->showStyledWarning(parent, "Success", "ailed to create process snapshot", true);
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
        parent->showStyledWarning(parent, "Success", "All Wizard101 clients have been terminated", false);
    }
    else {
        parent->showStyledWarning(parent, "Info", "No Wizard101 clients are running", false);
    }
}

void MiscManager::spoof() {
    // Work in Progress
    if (!spoofActive) {
        spoofActive = !spoofActive;
        ui->SpoofButton->setStyleSheet("background-color: green; color: white;");
        ui->SpoofButton->setText("Spoof (ON)");
    }
    else {
        spoofActive = !spoofActive;
        ui->SpoofButton->setStyleSheet("background-color: red; color: white;");
        ui->SpoofButton->setText("Spoof (OFF)");
    }
}

void MiscManager::browse() {
    if (ui->GameDropbox->currentText() == "Wizard101") {
        QString dir = QFileDialog::getExistingDirectory(parent, "Select Wizard101 Path");
        if (!dir.isEmpty()) {
            ui->inputWizardPath->setText(dir);
            *wizardPath = dir;
            savePathsToFile();
        }
    }
    else if (ui->GameDropbox->currentText() == "Pirate101") {
        QString dir = QFileDialog::getExistingDirectory(parent, "Select Pirate101 Path");
        if (!dir.isEmpty()) {
            ui->inputWizardPath->setText(dir);
            *piratePath = dir;
            savePathsToFile();
        }
    }
}

void MiscManager::gameSelect() {
    QString game = ui->GameDropbox->currentText();
    if (game == "Wizard101") {
        ui->Wizard101PathLabel->setText("Wizard101 Path");
        ui->inputWizardPath->setText(*wizardPath);

    }
    else if (game == "Pirate101") {
        ui->Wizard101PathLabel->setText("Pirate101 Path");
        ui->inputWizardPath->setText(*piratePath);
    }
}

void MiscManager::savePathsToFile() {
    QJsonObject paths;
    paths["wizard"] = *wizardPath;
    paths["pirate"] = *piratePath;
    (*jsonData)["paths"] = paths;
    parent->saveJson();
}


void MiscManager::prepareInjectDLL() {
    if (ui->injectDLLButton->text() == "Inject DLL") {
        QString filePath = QFileDialog::getOpenFileName(parent, "Select DLL to Inject", "", "DLL Files (*.dll)");
        if (!filePath.isEmpty()) {
            parent->setDLLPath(filePath);
            parent->showStyledWarning(parent, "DLL Selected", "DLL will be injected after game launch:\n" + filePath, false);
            ui->injectDLLButton->setText("Clear DLL");
            ui->injectDLLButton->setStyleSheet("background-color: green; color: white;");
        }
    }
    else {
        ui->injectDLLButton->setText("Inject DLL");
        ui->injectDLLButton->setStyleSheet("background-color: red; color: white;");
        parent->setDLLPath(nullptr);
        parent->showStyledWarning(parent, "DLL Cleared!", "DLL has been cleared from launcher, if you want to remove DLL's from current client(s) you must relaunch", false);
    }
}

void MiscManager::injectDLLToProcess(DWORD pid, QString& dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return;

    LPVOID allocMem = VirtualAllocEx(hProcess, nullptr, dllPath.size() * 2 + 2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!allocMem) {
        CloseHandle(hProcess);
        return;
    }

    WriteProcessMemory(hProcess, allocMem, dllPath.utf16(), dllPath.size() * 2, nullptr);

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
        (LPTHREAD_START_ROUTINE)LoadLibraryW, allocMem, 0, nullptr);

    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
    CloseHandle(hProcess);
}

