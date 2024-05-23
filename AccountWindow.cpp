#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <random>
#include "Header.h"

using namespace std;
using json = nlohmann::json;

json loadJson(const string& filePath) {
    ifstream inputFile(filePath);
    json data;
    if (inputFile.good()) {
        inputFile >> data;
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

void showAccountWindow(const string& userName, const string& accountId, const string& accountName, const string& currency) {
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

    json data = loadJson("loginData.json");
    string balance = "0";
    for (const auto& user : data["users"]) {
        if (user["name"] == userName) {
            for (const auto& account : user["accounts"]) {
                if (account["account_id"] == accountId) {
                    balance = account["balance"];
                    break;
                }
            }
            break;
        }
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

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
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
        window.display();
    }
}
