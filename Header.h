#ifndef HEADER_H
#define HEADER_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using std::string;

// Function declarations
json loadJson(const std::string& filePath);
void showSignUpWindow();
void showLoginWindow();
void showUserMainWindows(const string& userName);
void showCreateAccount(const string& userName);
void showAccountDetailsWindow(const string& userName, const string& accountId, const string& accountName, const string& currency); 
void showTransferDetailsWindow(const string& userName, const string& accountId, const string& accountName, string balance); 

#endif // HEADER_H