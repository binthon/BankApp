#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include "Header.h"

using namespace std;
using json = nlohmann::json;

json loadJsonTransfer(const std::string& filePath) {
    ifstream inputFile(filePath);
    json data;
    if (inputFile.good()) {
        inputFile >> data;
    }
    else {
        std::cout << "Failed to open file: " << filePath << endl;
    }
    return data;
}

vector<string> getTransferAccountNumbers(const json& data, const string& excludeAccountId) {
    vector<string> accountNumbers;
    for (const auto& user : data["users"]) {
        if (user.contains("accounts")) {
            for (const auto& account : user["accounts"]) {
                if (account.contains("account_id") && account["account_id"] != excludeAccountId) {
                    accountNumbers.push_back(account["account_id"]);
                }
            }
        }
    }
    return accountNumbers;
}

bool validateTransferPin(const string& userName, const string& enteredPin, const json& data) {
    for (const auto& user : data["users"]) {
        if (user["name"] == userName && user.contains("pin")) {
            return user["pin"] == enteredPin;
        }
    }
    return false;
}

void updateTransferBalanceInJson(const string& accountId, double amount) {
    json data = loadJsonTransfer("loginData.json");

    for (auto& user : data["users"]) {
        if (user.contains("accounts")) {
            for (auto& account : user["accounts"]) {
                if (account["account_id"] == accountId) {
                    double currentBalance = stod(account["balance"].get<string>());
                    currentBalance += amount;
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

void showTransferDetailsWindow(const string& userName, const string& accountId, const string& accountName, string balance) {
    sf::RenderWindow window(sf::VideoMode(1000, 600), "Transfer Details", sf::Style::Titlebar | sf::Style::Close);
    sf::Font font;

    if (!font.loadFromFile("arial.ttf")) {
        cout << "Could not load font\n";
        return;
    }

    sf::Text welcomeText("Welcome, " + userName, font, 20);
    welcomeText.setPosition(10, 10);
    welcomeText.setFillColor(sf::Color::White);

    sf::Text balanceText("Balance: " + balance, font, 20);
    balanceText.setPosition(10, 40);
    balanceText.setFillColor(sf::Color::White);

    sf::RectangleShape signOutButton(sf::Vector2f(150.f, 50.f));
    signOutButton.setPosition(850, 10);
    signOutButton.setFillColor(sf::Color::Red);

    sf::Text signOutText("Sign Out", font, 20);
    signOutText.setPosition(865, 20);
    signOutText.setFillColor(sf::Color::White);

    sf::Text infoText("Transfer from Account: " + accountName, font, 20);
    infoText.setPosition(10, 80);
    infoText.setFillColor(sf::Color::White);

    sf::Text setValueLabel("Set Value:", font, 20);
    setValueLabel.setPosition(10, 120);
    setValueLabel.setFillColor(sf::Color::White);

    sf::RectangleShape valueBox(sf::Vector2f(150.f, 30.f));
    valueBox.setPosition(150, 120);
    valueBox.setFillColor(sf::Color::White);
    valueBox.setOutlineColor(sf::Color::Black);
    valueBox.setOutlineThickness(2.f);

    sf::Text valueText("", font, 20);
    valueText.setPosition(155, 125);
    valueText.setFillColor(sf::Color::Black);

    sf::Text toAccountLabel("To Account ID:", font, 20);
    toAccountLabel.setPosition(10, 160);
    toAccountLabel.setFillColor(sf::Color::White);

    sf::RectangleShape toAccountBox(sf::Vector2f(300.f, 30.f));
    toAccountBox.setPosition(150, 160);
    toAccountBox.setFillColor(sf::Color::White);
    toAccountBox.setOutlineColor(sf::Color::Black);
    toAccountBox.setOutlineThickness(2.f);

    sf::Text toAccountText("", font, 20);
    toAccountText.setPosition(155, 165);
    toAccountText.setFillColor(sf::Color::Black);

    sf::Text pinLabel("PIN:", font, 20);
    pinLabel.setPosition(10, 200);
    pinLabel.setFillColor(sf::Color::White);

    sf::RectangleShape pinBox(sf::Vector2f(150.f, 30.f));
    pinBox.setPosition(150, 200);
    pinBox.setFillColor(sf::Color::White);
    pinBox.setOutlineColor(sf::Color::Black);
    pinBox.setOutlineThickness(2.f);

    sf::Text pinText("", font, 20);
    pinText.setPosition(155, 205);
    pinText.setFillColor(sf::Color::Black);

    sf::Text pinAsteriskText("", font, 20);
    pinAsteriskText.setPosition(155, 205);
    pinAsteriskText.setFillColor(sf::Color::Black);

    sf::RectangleShape confirmButton(sf::Vector2f(150.f, 50.f));
    confirmButton.setPosition(150, 250);
    confirmButton.setFillColor(sf::Color::Blue);

    sf::Text confirmButtonText("Confirm", font, 20);
    confirmButtonText.setPosition(175, 260);
    confirmButtonText.setFillColor(sf::Color::White);

    sf::RectangleShape backButton(sf::Vector2f(100.f, 50.f));
    backButton.setPosition(10, 530);
    backButton.setFillColor(sf::Color::Green);

    sf::Text backText("Back", font, 20);
    backText.setPosition(25, 540);
    backText.setFillColor(sf::Color::White);

    sf::Text choiceAccountText("Choice Account:", font, 20);
    choiceAccountText.setPosition(800, 210);
    choiceAccountText.setFillColor(sf::Color::White);

    json data = loadJsonTransfer("loginData.json");
    vector<string> accountNumbers = getTransferAccountNumbers(data, accountId);

    vector<sf::Text> accountTexts;
    float yPos = 250;
    for (const auto& accountNumber : accountNumbers) {
        sf::Text accountText(accountNumber, font, 12);
        accountText.setPosition(800, yPos);
        accountText.setFillColor(sf::Color::White);
        accountTexts.push_back(accountText);
        yPos += 30;  // Increase the gap between account numbers

        // Draw a line separator
        sf::RectangleShape lineSeparator(sf::Vector2f(180.f, 2.f));
        lineSeparator.setPosition(800, yPos - 10);
        lineSeparator.setFillColor(sf::Color::White);
        accountTexts.push_back(sf::Text("", font, 12));  // Add a placeholder for the line
    }

    string valueInput;
    string selectedAccount;
    string pinInput;
    string pinAsterisk;

    bool valueActive = false;
    bool pinActive = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::TextEntered) {
                if (valueActive && isdigit(static_cast<char>(event.text.unicode)) && valueInput.size() < 5) {
                    valueInput += static_cast<char>(event.text.unicode); // Explicit cast
                    valueText.setString(valueInput);
                }
                else if (valueActive && event.text.unicode == '\b' && !valueInput.empty()) {
                    valueInput.pop_back();
                    valueText.setString(valueInput);
                }

                if (pinActive && isdigit(static_cast<char>(event.text.unicode)) && pinInput.size() < 4) {
                    pinInput += static_cast<char>(event.text.unicode); // Explicit cast
                    pinAsterisk += '*';
                    pinAsteriskText.setString(pinAsterisk);
                }
                else if (pinActive && event.text.unicode == '\b' && !pinInput.empty()) {
                    pinInput.pop_back();
                    pinAsterisk.pop_back();
                    pinAsteriskText.setString(pinAsterisk);
                }
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (backButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        window.close();
                        showAccountDetailsWindow(userName, accountId, accountName, balance);
                    }
                    else if (signOutButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        window.close();
                        showLoginWindow();
                    }
                    else if (confirmButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        if (!valueInput.empty() && !selectedAccount.empty() && !pinInput.empty()) {
                            double amount = stod(valueInput);
                            if (amount <= stod(balance)) {
                                if (validateTransferPin(userName, pinInput, data)) {

                                    cout << "Transfer " << amount << " from " << accountId << " to " << selectedAccount << endl;
                                    updateTransferBalanceInJson(selectedAccount, amount);
                                    updateTransferBalanceInJson(accountId, -amount);
                                    balance = to_string(stod(balance) - amount);
                                    balanceText.setString("Balance: " + balance);


                                    valueInput.clear();
                                    selectedAccount.clear();
                                    pinInput.clear();
                                    pinAsterisk.clear();
                                    valueText.setString("");
                                    toAccountText.setString("");
                                    pinAsteriskText.setString("");
                                }
                                else {
                                    cout << "Invalid PIN." << endl;
                                }
                            }
                            else {
                                cout << "Amount exceeds balance." << endl;
                            }
                        }
                        else {
                            cout << "Enter amount, select an account, and enter PIN." << endl;
                        }
                    }
                    else {
                        for (size_t i = 0; i < accountTexts.size(); ++i) {
                            if (accountTexts[i].getString() != "" && accountTexts[i].getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                                selectedAccount = accountNumbers[i / 2];  // accountNumbers index should be half of accountTexts index
                                toAccountText.setString(selectedAccount);
                                break;
                            }
                        }
                    }

                    valueActive = valueBox.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y);
                    pinActive = pinBox.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y);
                }
            }
        }

        window.clear(sf::Color::Black);
        window.draw(welcomeText);
        window.draw(balanceText);
        window.draw(infoText);
        window.draw(setValueLabel);
        window.draw(valueBox);
        window.draw(valueText);
        window.draw(toAccountLabel);
        window.draw(toAccountBox);
        window.draw(toAccountText);
        window.draw(pinLabel);
        window.draw(pinBox);
        window.draw(pinAsteriskText);
        window.draw(confirmButton);
        window.draw(confirmButtonText);
        window.draw(signOutButton);
        window.draw(signOutText);
        window.draw(backButton);
        window.draw(backText);
        window.draw(choiceAccountText);

        for (size_t i = 0; i < accountTexts.size(); ++i) {
            window.draw(accountTexts[i]);
            if (i % 2 != 0) {  // Draw the line separator
                sf::RectangleShape lineSeparator(sf::Vector2f(180.f, 2.f));
                lineSeparator.setPosition(800, accountTexts[i - 1].getPosition().y + 20);
                lineSeparator.setFillColor(sf::Color::White);
                window.draw(lineSeparator);
            }
        }

        window.display();
    }
}
