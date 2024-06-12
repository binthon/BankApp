#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <random>
#include "Header.h"

using namespace std;
using json = nlohmann::json;

struct TextField2 {
    sf::RectangleShape box;
    sf::Text text;
    string input;
    bool isActive = false;
    bool isEditable = true;

    TextField2(float x, float y, const sf::Font& font, unsigned int fontSize = 20, bool editable = true)
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
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Create Account", sf::Style::Titlebar | sf::Style::Close);
    sf::Font font;

    if (!font.loadFromFile("TTFors.ttf")) {
        cout << "Could not load font\n";
        return;
    }

    sf::Texture logoTexture;
    if (!logoTexture.loadFromFile("logo.png")) {
        cerr << "Failed to load logo texture\n";
        return;
    }
    sf::Sprite logoSprite;
    logoSprite.setTexture(logoTexture);
    logoSprite.setPosition(window.getSize().x / 2 - logoSprite.getLocalBounds().width / 2, -180);

    sf::Texture frameTexture;
    if (!frameTexture.loadFromFile("frame.png")) {
        cerr << "Failed to load frame texture\n";
        return;
    }

    sf::Texture buttonTexture;
    if (!buttonTexture.loadFromFile("save_button.png")) {
        cerr << "Failed to load button texture\n";
        return;
    }

    sf::Sprite saveAccountButton(buttonTexture);
    saveAccountButton.setPosition(window.getSize().x / 2 - 75, 600);

    sf::Texture backbuttonTexture;
    if (!backbuttonTexture.loadFromFile("back_button.png")) {
        cerr << "Failed to load button texture\n";
        return;
    }

    sf::Sprite backButton(backbuttonTexture);
    backButton.setPosition(window.getSize().x / 2 - 75, 700);

    sf::Sprite signOutButton(backbuttonTexture);
    signOutButton.setPosition(1750, 50);

    sf::Text signOutText("Sign Out", font, 20);
    signOutText.setPosition(signOutButton.getPosition().x + 45, signOutButton.getPosition().y + 12);
    signOutText.setFillColor(sf::Color::White);

    sf::Text saveAccountText("Save Account", font, 20);
    saveAccountText.setPosition(saveAccountButton.getPosition().x + 30, saveAccountButton.getPosition().y + 15);
    saveAccountText.setFillColor(sf::Color::White);

    sf::Text backText("Back", font, 20);
    backText.setPosition(backButton.getPosition().x + 70, backButton.getPosition().y + 15);
    backText.setFillColor(sf::Color::White);


    sf::Text welcomeText("Welcome, " + userName, font, 30);
    welcomeText.setPosition(1470, 60);
    welcomeText.setFillColor(sf::Color::White);

    sf::Text accountNameLabel("Account Name:", font, 20);
    accountNameLabel.setPosition(window.getSize().x / 2 - 200, 300);
    accountNameLabel.setFillColor(sf::Color::White);

    sf::Text accountIdLabel("Account ID:", font, 20);
    accountIdLabel.setPosition(window.getSize().x / 2 - 200, 350);
    accountIdLabel.setFillColor(sf::Color::White);

    sf::Text currencyAccountLabel("Currency Account:", font, 20);
    currencyAccountLabel.setPosition(window.getSize().x / 2 - 200, 400);
    currencyAccountLabel.setFillColor(sf::Color::White);

    sf::Text currencyLabel("Currency:", font, 20);
    currencyLabel.setPosition(window.getSize().x / 2 - 200, 450);
    currencyLabel.setFillColor(sf::Color::White);

    TextField2 accountNameField(window.getSize().x / 2, 300, font);
    TextField2 balanceField(window.getSize().x / 2, 500, font);
    balanceField.setText("0");

    string accountId = generateRandomAccountId();
    sf::Text accountIdText(accountId, font, 20);
    accountIdText.setPosition(window.getSize().x / 2, 350);
    accountIdText.setFillColor(sf::Color::White);

    sf::RectangleShape yesButton(sf::Vector2f(80.f, 30.f));
    yesButton.setPosition(window.getSize().x / 2, 400);
    yesButton.setFillColor(sf::Color::White);

    sf::Text yesText("Yes", font, 20);
    yesText.setPosition(window.getSize().x / 2 + 10, 405);
    yesText.setFillColor(sf::Color::Black);

    sf::RectangleShape noButton(sf::Vector2f(80.f, 30.f));
    noButton.setPosition(window.getSize().x / 2 + 100, 400);
    noButton.setFillColor(sf::Color::Green);

    sf::Text noText("No", font, 20);
    noText.setPosition(window.getSize().x / 2 + 110, 405);
    noText.setFillColor(sf::Color::Black);

    bool isYesSelected = false;
    bool isNoSelected = true;

    vector<sf::RectangleShape> currencyButtons;
    vector<sf::Text> currencyTexts;
    vector<string> currencies = { "PLN", "USD", "EUR", "GBP", "AUD", "CAD", "CHF", "JPY", "CNY" };
    float startX = window.getSize().x / 2, startY = 450;

    for (size_t i = 0; i < currencies.size(); ++i) {
        sf::RectangleShape button(sf::Vector2f(60.f, 30.f));
        button.setPosition(startX + (i % 3) * 70, startY + (i / 3) * 40);
        button.setFillColor(i == 0 ? sf::Color::Green : sf::Color::White);
        currencyButtons.push_back(button);

        sf::Text buttonText(currencies[i], font, 20);
        buttonText.setPosition(button.getPosition().x + 10, button.getPosition().y + 5);
        buttonText.setFillColor(sf::Color::Black);
        currencyTexts.push_back(buttonText);
    }

    int selectedCurrencyIndex = 0;
    sf::Text successText("", font, 20);
    successText.setPosition(window.getSize().x / 2 - 120, 800);
    successText.setFillColor(sf::Color::Green);

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
                    yesButton.setFillColor(sf::Color::Green);
                    noButton.setFillColor(sf::Color::White);
                    selectedCurrencyIndex = 1;
                }
                else if (noButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    isYesSelected = false;
                    isNoSelected = true;
                    yesButton.setFillColor(sf::Color::White);
                    noButton.setFillColor(sf::Color::Green);
                    selectedCurrencyIndex = 0;
                }
                else {
                    accountNameField.setActive(accountNameField.box.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y));
                    balanceField.setActive(balanceField.box.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y));

                    if (signOutButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        window.close();
                        showLoginWindow();
                    }
                    else if (backButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
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
                            successText.setString("Account created successfully!");
                            cout << "Account saved!" << endl;
                        }
                        else {
                            successText.setString("");
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
                currencyButtons[i].setFillColor(sf::Color::Green);
            }
            else {
                currencyButtons[i].setFillColor(sf::Color::White);
            }
        }

        window.clear(sf::Color(0x38, 0xB6, 0xFF));

        window.draw(logoSprite);
        window.draw(saveAccountButton);
        window.draw(saveAccountText);
        window.draw(signOutButton);
        window.draw(signOutText);
        window.draw(backButton);
        window.draw(backText);
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
        window.draw(successText);
        window.display();
    }
}
