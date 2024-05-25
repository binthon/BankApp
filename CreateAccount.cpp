#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <random>
#include "Header.h"

using namespace std;
using json = nlohmann::json;

struct TextField {
    sf::RectangleShape box;
    sf::Text text;
    string input;
    bool isActive = false;
    bool isEditable = true;

    TextField(float x, float y, const sf::Font& font, unsigned int fontSize = 20, bool editable = true)
        : isEditable(editable) {
        box.setSize(sf::Vector2f(200.f, 30.f));
        box.setPosition(x, y);
        box.setFillColor(sf::Color::White);
        box.setOutlineColor(sf::Color::Black);
        box.setOutlineThickness(2.f);

        text.setFont(font);
        text.setCharacterSize(fontSize);
        text.setPosition(x + 5, y + 5);
        text.setFillColor(sf::Color::Black);
    }

    void setActive(bool active) {
        isActive = active;
        if (isActive) {
            box.setOutlineColor(sf::Color::Blue);
        }
        else {
            box.setOutlineColor(sf::Color::Black);
        }
    }

    void handleEvent(sf::Event event) {
        if (event.type == sf::Event::TextEntered && isEditable) {
            if (isActive) {
                if (event.text.unicode == '\b' && !input.empty()) {
                    input.pop_back();
                }
                else if (event.text.unicode < 128 && event.text.unicode != '\b') {
                    input += static_cast<char>(event.text.unicode);
                }
                text.setString(input);
            }
        }
    }

    void setText(const string& newText) {
        input = newText;
        text.setString(newText);
    }
};

string generateRandomAccountId() {
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<int> distribution(0, 9);
    string accountId;

    for (int i = 0; i < 26; ++i) {
        accountId += to_string(distribution(generator));
    }

    return accountId;
}

bool accountExists(const json& user, const string& accountName) {
    if (user.contains("accounts")) {
        for (const auto& account : user["accounts"]) {
            if (account["account_name"] == accountName) {
                return true;
            }
        }
    }
    return false;
}

void saveAccountData(const string& userName, const string& accountName, const string& accountId, const string& currency, const string& balance) {
    json jsonData;
    ifstream inFile("loginData.json");
    if (inFile.is_open()) {
        inFile >> jsonData;
        inFile.close();
    }

    if (!jsonData.contains("users")) {
        cout << "No users found in the JSON data." << endl;
        return;
    }

    for (auto& user : jsonData["users"]) {
        if (user["name"] == userName) {
            if (accountExists(user, accountName)) {
                cout << "Account with this name already exists!" << endl;
                return;
            }

            if (!user.contains("accounts")) {
                user["accounts"] = json::array();
            }

            json newAccount = {
                {"id", user["accounts"].size() + 1},
                {"account_id", accountId},
                {"account_name", accountName},
                {"currency", currency},
                {"balance", balance}
            };
            user["accounts"].push_back(newAccount);
            break;
        }
    }

    ofstream outFile("loginData.json");
    if (outFile.is_open()) {
        outFile << jsonData.dump(4);
        outFile.close();
    }
    else {
        cout << "Could not open file for writing." << endl;
    }
}

void showCreateAccount(const string& userName) {
    sf::RenderWindow window(sf::VideoMode(1000, 500), "Create Account", sf::Style::Titlebar | sf::Style::Close);
    sf::Font font;

    if (!font.loadFromFile("arial.ttf")) {
        cout << "Could not load font\n";
        return;
    }

    sf::RectangleShape saveAccountButton(sf::Vector2f(150.f, 50.f));
    saveAccountButton.setPosition(425, 430);
    saveAccountButton.setFillColor(sf::Color::Green);

    sf::Text saveAccountText("Save Account", font, 20);
    saveAccountText.setPosition(440, 445);
    saveAccountText.setFillColor(sf::Color::White);

    sf::RectangleShape signOutButton(sf::Vector2f(100.f, 50.f));
    signOutButton.setPosition(880, 10);
    signOutButton.setFillColor(sf::Color::Red);

    sf::Text signOutText("Sign Out", font, 20);
    signOutText.setPosition(885, 25);
    signOutText.setFillColor(sf::Color::White);

    sf::RectangleShape mainWindowButton(sf::Vector2f(200.f, 50.f));
    mainWindowButton.setPosition(10, 430);
    mainWindowButton.setFillColor(sf::Color::Blue);

    sf::Text mainWindowButtonText("Main Window", font, 20);
    mainWindowButtonText.setPosition(20, 445);
    mainWindowButtonText.setFillColor(sf::Color::White);

    sf::Text accountNameLabel("Account Name:", font, 20);
    accountNameLabel.setPosition(300, 50);
    accountNameLabel.setFillColor(sf::Color::White);

    sf::Text accountIdLabel("Account ID:", font, 20);
    accountIdLabel.setPosition(300, 100);
    accountIdLabel.setFillColor(sf::Color::White);

    sf::Text currencyAccountLabel("Currency Account:", font, 20);
    currencyAccountLabel.setPosition(300, 150);
    currencyAccountLabel.setFillColor(sf::Color::White);

    sf::Text currencyLabel("Currency:", font, 20);
    currencyLabel.setPosition(300, 200);
    currencyLabel.setFillColor(sf::Color::White);

    TextField accountNameField(500, 50, font);
    TextField balanceField(500, 250, font);
    balanceField.setText("0"); // Set default balance to 0

    string accountId = generateRandomAccountId();
    sf::Text accountIdText(accountId, font, 20);
    accountIdText.setPosition(500, 100);
    accountIdText.setFillColor(sf::Color::White);

    sf::Text welcomeText("Welcome, " + userName, font, 20);
    welcomeText.setPosition(10, 10);
    welcomeText.setFillColor(sf::Color::White);

    sf::RectangleShape yesButton(sf::Vector2f(80.f, 30.f));
    yesButton.setPosition(500, 150);
    yesButton.setFillColor(sf::Color::Green);

    sf::Text yesText("Yes", font, 20);
    yesText.setPosition(510, 150);
    yesText.setFillColor(sf::Color::Black);

    sf::RectangleShape noButton(sf::Vector2f(80.f, 30.f));
    noButton.setPosition(600, 150);
    noButton.setFillColor(sf::Color::Yellow);

    sf::Text noText("No", font, 20);
    noText.setPosition(610, 150);
    noText.setFillColor(sf::Color::Black);

    bool isYesSelected = false;
    bool isNoSelected = true;

    vector<sf::RectangleShape> currencyButtons;
    vector<sf::Text> currencyTexts;
    vector<string> currencies = { "PLN", "USD", "EUR", "GBP", "AUD", "CAD", "CHF", "JPY", "CNY" };
    float startX = 500, startY = 200;

    for (size_t i = 0; i < currencies.size(); ++i) {
        sf::RectangleShape button(sf::Vector2f(60.f, 30.f));
        button.setPosition(startX + (i % 3) * 70, startY + (i / 3) * 40);
        button.setFillColor(i == 0 ? sf::Color::Yellow : sf::Color::White);
        currencyButtons.push_back(button);

        sf::Text buttonText(currencies[i], font, 20);
        buttonText.setPosition(button.getPosition().x + 10, button.getPosition().y + 5);
        buttonText.setFillColor(sf::Color::Black);
        currencyTexts.push_back(buttonText);
    }

    int selectedCurrencyIndex = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (yesButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isYesSelected = true;
                    isNoSelected = false;
                    yesButton.setFillColor(sf::Color::Yellow);
                    noButton.setFillColor(sf::Color::Red);
                    selectedCurrencyIndex = 1;  // Set default currency to USD when Yes is selected
                }
                else if (noButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isYesSelected = false;
                    isNoSelected = true;
                    yesButton.setFillColor(sf::Color::Green);
                    noButton.setFillColor(sf::Color::Yellow);
                    selectedCurrencyIndex = 0;  // Set default currency to PLN when No is selected
                }
                else {
                    accountNameField.setActive(accountNameField.box.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y));
                    balanceField.setActive(balanceField.box.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y));

                    if (signOutButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        window.close();
                        showLoginWindow();
                    }
                    else if (mainWindowButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        window.close();
                        showUserMainWindows(userName);
                    }

                    if (isYesSelected) {
                        for (size_t i = 0; i < currencyButtons.size(); ++i) {
                            if (currencyButtons[i].getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                                selectedCurrencyIndex = i;
                            }
                        }
                    }

                    if (saveAccountButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        if (!accountNameField.input.empty() && !balanceField.input.empty()) {
                            string selectedCurrency = isYesSelected ? currencies[selectedCurrencyIndex] : "PLN";
                            saveAccountData(userName, accountNameField.input, accountId, selectedCurrency, balanceField.input);
                            accountId = generateRandomAccountId();
                            accountIdText.setString(accountId);
                            cout << "Account saved!" << endl;
                        }
                        else {
                            cout << "Account Name or Balance field is empty!" << endl;
                        }
                    }
                }
            }

            accountNameField.handleEvent(event);
            balanceField.handleEvent(event);
        }

        for (size_t i = 0; i < currencyButtons.size(); ++i) {
            if (i == selectedCurrencyIndex) {
                currencyButtons[i].setFillColor(sf::Color::Yellow);
            }
            else {
                currencyButtons[i].setFillColor(sf::Color::White);
            }
        }

        window.clear(sf::Color::Black);
        window.draw(saveAccountButton);
        window.draw(saveAccountText);
        window.draw(signOutButton);
        window.draw(signOutText);
        window.draw(mainWindowButton);
        window.draw(mainWindowButtonText);
        window.draw(accountNameLabel);
        window.draw(accountIdLabel);
        window.draw(currencyAccountLabel);
        window.draw(currencyLabel);
        window.draw(accountNameField.box);
        window.draw(accountNameField.text);
        window.draw(accountIdText);
        window.draw(yesButton);
        window.draw(yesText);
        window.draw(noButton);
        window.draw(noText);

        if (isYesSelected) {
            for (size_t i = 1; i < currencyButtons.size(); ++i) {
                window.draw(currencyButtons[i]);
                window.draw(currencyTexts[i]);
            }
        }
        else {
            window.draw(currencyButtons[0]);
            window.draw(currencyTexts[0]);
        }

        window.draw(welcomeText);
        window.display();
    }
}
