# KingsisleLauncher | Wizard101 and Pirate101

[Qt](https://doc.qt.io/qt-6/get-and-install-qt.html) is a free and open-source widget toolkit for creating graphical user interfaces as well as cross-platform applications that run on various software and hardware platforms with little or no change in the underlying codebase
https://doc.qt.io/qt-6/get-and-install-qt.html

## What is this project

Wizard101 launcher acts as a file integiry checker and an update manager which would may take about ~3 minutes to login for each account.

By directly launching `"C:\Wizard101\Bin\WizardGraphicalClient.exe" -L login.us.wizard101.com 12000` the quick launcher will skip the file itegrity and patching process assuming there havent been an update recently. The game executable (WizardGraphicalClient.exe) expects to be launched this way. It has no built-in login UI so it’s designed to only run when a trusted parent (like Wizard101GraphicalClient.exe) gives it a verified session token and required launch flags. But the launcher doesn't do any OS-level validation — so if you know the correct parameters and format, you can mimic the launcher yourself.

`-L` is a launch flag that tells the client to connect to a login server manually. most likely a hardcoded debug/developer parameter inside the game binary

`login.us.wizard101.com` The hostname of the Wizard101 US login server where the game authenticates your login

`12000` standard TCP port that kingsisle uses for all their games

![QuickLauncher Screenshot](KingsisleLauncher/images/login.png) 

## Work in Progress
This project is a work in progress and will keep evolving over time (whenever I feel like it). Features are being added and tweaked on the fly, with only the client launcher working

## Why it was made

This launcher was inspired by MilkLauncher, a Python-based tool created for launching Wizard101 & Pirate101. While MilkLauncher was a great tool, it didn't fully meet the needs of my group between me and a few others. we needed a solution that allowed precise control, launching only the accounts I needed at any given time. I don't expect many people if anyone'to use this, but it exists for this scenario.

## Top Section
This is used for single use logins:

Nickname      - A custom label you assign to each account. This helps you quickly identify which account is which inside the launcher

Username      - In game username

Password      - In game password

Launch button - used to launch the client that is currently selected in the top section dropdown
All account information is saved and read from files stored in the /information directory. These files are automatically loaded at startup

## Middle Section
This is used for people with many many accounts and need to sort to be sorted for certain situations. This sections allows user to store them in "Bundles" of your choosing so esentially creating a multi launch preset.

Bundle Launch   - used to launch the client that is currently selected in the middle section dropdown

Bundle Nickname - used as a personal preference name to remind you of which accounts are inside the bundle

Mass Nickname   - This will use the nicknames from the top section to allow you to bundle accounts into 1 and launch at the same time
	eg. "StormHitter/LifeJade/IceTank/FireHitter"

## Bottom Section
KillAll   - Instantly `hard` terminates all running Wizard101 clients meaning The game doesn't finish its logout logic On next login, the client assumes you're still in your last loaded state

InjectDLL - Prompts the user to browse for a DLL file, which will then be automatically be injected into each launched client <span style="color: red;">WORK IN PROGRESS</span>

Spoof     - When enabled, the launcher will spoof HWID and IP  <span style="color: red;">WORK IN PROGRESS</span>

# How the autolaunch works
When you launch an account with the program:
1. The launcher constructs the appropriate launch command based on the selected game `-L login.us.wizard101.com 12000`
2. It starts the game's executable using `CreateProcessW`
3. After a short delay to allow the game to start up, it finds the main window via `FindWindowW` for input
4. The launcher simulates keyboard input based on the users username and password

UI Preview

![QuickLauncher Screenshot](KingsisleLauncher/images/empty.png)

![QuickLauncher Screenshot](KingsisleLauncher/images/full.png)
