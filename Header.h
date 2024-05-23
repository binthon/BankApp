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

#endif // HEADER_H
