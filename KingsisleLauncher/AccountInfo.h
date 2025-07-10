#ifndef ACCOUNTINFO_H
#define ACCOUNTINFO_H

#include <QString>

struct AccountInfo {
    QString nickname;
    QString username;
    QString password;
};

struct BundleInfo {
    QString bundleNickname;
    QString massBundle;
};

#endif