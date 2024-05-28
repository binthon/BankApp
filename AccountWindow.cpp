#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include "Header.h"


using namespace std;
using json = nlohmann::json;

json loadJson(const string& filePath) {
    ifstream inputFile(filePath);
    json data;
    if (inputFile.good()) {
        inputFile >> data;
    }
    else {
        cout << "Failed to open file: " << filePath << endl;
    }
    return data;
}

vector<json> getAccounts(const string& userName, json& data) {
    for (auto& user : data["users"]) {
        if (user["name"] == userName) {
            if (!user.contains("accounts")) {
                user["accounts"] = json::array();
            }
            return user["accounts"].get<vector<json>>();
        }
    }
    return {};
}

bool hasPin(const string& userName, const json& data) {
    for (const auto& user : data["users"]) {
        if (user["name"] == userName && user.contains("pin")) {
            return true;
        }
    }
    return false;
}

void updateBalanceInJson(const string& userName, const string& accountId, double amount, bool isPay) {
    json data;
    try {
        data = loadJson("loginData.json");
    }
    catch (const json::parse_error& e) {
        cout << "JSON parse error: " << e.what() << endl;
        return;
    }

    for (auto& user : data["users"]) {
        if (user["name"] == userName) {
            if (user.contains("accounts")) {
                for (auto& account : user["accounts"]) {
                    if (account["account_id"] == accountId) {
                        double currentBalance = stod(account["balance"].get<string>());
                        if (isPay) {
                            currentBalance += amount;
                        }
                        else {
                            if (currentBalance < amount) {
                                cout << "Insufficient balance for payout." << endl;
                                return;
                            }
                            currentBalance -= amount;
                        }
                        account["balance"] = to_string(currentBalance);
                        ofstream outFile("loginData.json");
                        if (outFile.is_open()) {
                            outFile << data.dump(4);
                            outFile.close();
                        }
                        else {
                            cout << "Could not open file for writing." << endl;
                        }
                        return;
                    }
                }
            }
        }
    }
}

void showAccountDetailsWindow(const string& userName, const string& accountId, const string& accountName, const string& currency) {
    sf::RenderWindow window(sf::VideoMode(1000, 500), "Account Details", sf::Style::Titlebar | sf::Style::Close);
    sf::Font font;

    if (!font.loadFromFile("arial.ttf")) {
        cout << "Could not load font\n";
        return;
    }

    sf::Text welcomeText("Welcome, " + userName, font, 20);
    welcomeText.setPosition(10, 10);
    welcomeText.setFillColor(sf::Color::White);

    sf::RectangleShape signOutButton(sf::Vector2f(100.f, 50.f));
    signOutButton.setPosition(880, 10);
    signOutButton.setFillColor(sf::Color::Red);

    sf::Text signOutText("Sign Out", font, 20);
    signOutText.setPosition(890, 20);
    signOutText.setFillColor(sf::Color::White);

    sf::Text accountIdLabel("Account ID: " + accountId, font, 20);
    accountIdLabel.setPosition(10, 50);
    accountIdLabel.setFillColor(sf::Color::White);

    sf::Text accountNameLabel("Account Name: " + accountName, font, 20);
    accountNameLabel.setPosition(10, 90);
    accountNameLabel.setFillColor(sf::Color::White);

    sf::Text currencyLabel("Currency: " + currency, font, 20);
    currencyLabel.setPosition(10, 130);
    currencyLabel.setFillColor(sf::Color::White);

    json data;
    try {
        data = loadJson("loginData.json");
    }
    catch (const json::parse_error& e) {
        cout << "JSON parse error: " << e.what() << endl;
        return;
    }

    string balance = "0";
    bool accountFound = false;
    for (const auto& user : data["users"]) {
        if (user.contains("name") && user["name"] == userName) {
            if (user.contains("accounts")) {
                for (const auto& account : user["accounts"]) {
                    if (account.contains("account_id") && account["account_id"] == accountId) {
                        balance = account.value("balance", "0");
                        accountFound = true;
                        break;
                    }
                }
            }
            break;
        }
    }

    if (!accountFound) {
        cout << "Account not found for user: " << userName << endl;
        return;
    }

    sf::Text balanceLabel("Balance: " + balance, font, 20);
    balanceLabel.setPosition(10, 170);
    balanceLabel.setFillColor(sf::Color::White);

    sf::RectangleShape backButton(sf::Vector2f(100.f, 50.f));
    backButton.setPosition(10, 430);
    backButton.setFillColor(sf::Color::Green);

    sf::Text backText("Back", font, 20);
    backText.setPosition(25, 440);
    backText.setFillColor(sf::Color::White);

    sf::RectangleShape transferButton(sf::Vector2f(200.f, 50.f));
    transferButton.setPosition(780, 100);
    transferButton.setFillColor(sf::Color::Blue);

    sf::Text transferText("Transfer", font, 20);
    transferText.setPosition(815, 115);
    transferText.setFillColor(sf::Color::White);

    sf::RectangleShape depositButton(sf::Vector2f(200.f, 50.f));
    depositButton.setPosition(780, 170);
    depositButton.setFillColor(sf::Color::Blue);

    sf::Text depositText("Deposit", font, 20);
    depositText.setPosition(815, 185);
    depositText.setFillColor(sf::Color::White);

    sf::RectangleShape amountBox(sf::Vector2f(150.f, 30.f));
    amountBox.setPosition(780, 230);
    amountBox.setFillColor(sf::Color::White);
    amountBox.setOutlineColor(sf::Color::Black);
    amountBox.setOutlineThickness(2.f);

    sf::Text amountText("", font, 20);
    amountText.setPosition(785, 235);
    amountText.setFillColor(sf::Color::Black);

    sf::RectangleShape payButton(sf::Vector2f(100.f, 50.f));
    payButton.setPosition(680, 280);
    payButton.setFillColor(sf::Color::Green);

    sf::Text payText("Pay", font, 20);
    payText.setPosition(705, 295);
    payText.setFillColor(sf::Color::White);

    sf::RectangleShape payoutButton(sf::Vector2f(100.f, 50.f));
    payoutButton.setPosition(830, 280);
    payoutButton.setFillColor(sf::Color::Red);

    sf::Text payoutText("Payout", font, 20);
    payoutText.setPosition(845, 295);
    payoutText.setFillColor(sf::Color::White);

    sf::Text errorText("", font, 20);
    errorText.setPosition(400, 300);
    errorText.setFillColor(sf::Color::Red);

    string amountInput;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::TextEntered) {
                if (isdigit(event.text.unicode) && amountInput.size() < 5) {
                    amountInput += event.text.unicode;
                    amountText.setString(amountInput);
                }
                else if (event.text.unicode == '\b' && !amountInput.empty()) {
                    amountInput.pop_back();
                    amountText.setString(amountInput);
                }
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (backButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        window.close();
                        showUserMainWindows(userName);
                    }
                    else if (signOutButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        window.close();
                        showLoginWindow();
                    }
                    else if (transferButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        if (hasPin(userName, data)) {
                            window.close();
                            showTransferDetailsWindow(userName, accountId, accountName, balance); // Updated function name
                        }
                        else {
                            errorText.setString("You need to set a PIN to make a transfer.");
                        }
                    }
                    else if (payButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y) && !amountInput.empty()) {
                        double amount = stod(amountInput);
                        updateBalanceInJson(userName, accountId, amount, true);
                        balance = to_string(stod(balance) + amount);
                        balanceLabel.setString("Balance: " + balance);
                    }
                    else if (payoutButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y) && !amountInput.empty()) {
                        double amount = stod(amountInput);
                        if (stod(balance) >= amount) {
                            updateBalanceInJson(userName, accountId, amount, false);
                            balance = to_string(stod(balance) - amount);
                            balanceLabel.setString("Balance: " + balance);
                        }
                        else {
                            errorText.setString("Insufficient balance for payout.");
                        }
                    }
                }
            }
        }

        window.clear(sf::Color::Black);
        window.draw(welcomeText);
        window.draw(signOutButton);
        window.draw(signOutText);
        window.draw(accountIdLabel);
        window.draw(accountNameLabel);
        window.draw(currencyLabel);
        window.draw(balanceLabel);
        window.draw(backButton);
        window.draw(backText);
        window.draw(transferButton);
        window.draw(transferText);
        window.draw(depositButton);
        window.draw(depositText);
        window.draw(amountBox);
        window.draw(amountText);
        window.draw(payButton);
        window.draw(payText);
        window.draw(payoutButton);
        window.draw(payoutText);
        window.draw(errorText);
        window.display();
    }
}
