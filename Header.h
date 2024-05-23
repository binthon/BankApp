#pragma once
#ifndef HEADER_H
#define HEADER_H

#include <string>

using std::string;

// Function declarations
void showSignUpWindow();
void showLoginWindow();
void showUserMainWindows(const string& userName);
void showCreateAccount(const string& userName);
void showAccountWindow(const string& userName, const string& accountId, const string& accountName, const string& currency);

#endif // HEADER_H
