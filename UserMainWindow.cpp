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

json loadJsonData(const string& filePath) {
    ifstream inputFile(filePath);
    json data;
    if (inputFile.good()) {
        inputFile >> data;
    }
    return data;
}

vector<json> getUserAccounts(const string& userName, json& data) {
    for (auto& user : data["users"]) {
        if (user["name"] == userName) {
            if (!user.contains("accounts")) {
                user["accounts"] = json::array(); // Ensure accounts field exists
            }
            return user["accounts"].get<vector<json>>();
        }
    }
    return {};
}


void showUserMainWindows(const string& userName) {
    sf::RenderWindow window(sf::VideoMode(1000, 500), "User Main Window", sf::Style::Titlebar | sf::Style::Close);
    sf::Font font;

    if (!font.loadFromFile("arial.ttf")) {
        cout << "Could not load font\n";
        return;
    }

    sf::RectangleShape createAccountButton(sf::Vector2f(200.f, 50.f));
    createAccountButton.setPosition(400, 430);
    createAccountButton.setFillColor(sf::Color::Green);

    sf::Text createAccountText("Create Account", font, 20);
    createAccountText.setPosition(415, 445);
    createAccountText.setFillColor(sf::Color::White);

    sf::RectangleShape signOutButton(sf::Vector2f(100.f, 50.f));
    signOutButton.setPosition(880, 10);
    signOutButton.setFillColor(sf::Color::Red);

    sf::Text signOutText("Sign Out", font, 20);
    signOutText.setPosition(885, 25);
    signOutText.setFillColor(sf::Color::White);

    sf::Text welcomeText("Welcome, " + userName, font, 20);
    welcomeText.setPosition(10, 10);
    welcomeText.setFillColor(sf::Color::White);

    json data = loadJsonData("loginData.json");
    vector<json> accounts = getUserAccounts(userName, data);

    int currentPage = 0;
    const int accountsPerPage = 4;
    int totalPages = (accounts.size() + accountsPerPage - 1) / accountsPerPage;

    sf::RectangleShape nextButton(sf::Vector2f(50.f, 50.f));
    nextButton.setPosition(730, 430);
    nextButton.setFillColor(sf::Color::Blue);

    sf::Text nextText(">", font, 30);
    nextText.setPosition(745, 435);
    nextText.setFillColor(sf::Color::White);

    sf::RectangleShape prevButton(sf::Vector2f(50.f, 50.f));
    prevButton.setPosition(220, 430);
    prevButton.setFillColor(sf::Color::Blue);

    sf::Text prevText("<", font, 30);
    prevText.setPosition(235, 435);
    prevText.setFillColor(sf::Color::White);

    vector<sf::RectangleShape> accountBlocks;
    for (int i = 0; i < accounts.size(); ++i) {
        sf::RectangleShape accountBlock(sf::Vector2f(400.f, 80.f));
        accountBlocks.push_back(accountBlock);
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (signOutButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        window.close();
                        showLoginWindow();
                    }
                    else if (createAccountButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        window.close();
                        showCreateAccount(userName);
                    }
                    else if (nextButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y) && currentPage < totalPages - 1) {
                        currentPage++;
                    }
                    else if (prevButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y) && currentPage > 0) {
                        currentPage--;
                    }
                    else {
                        int startIdx = currentPage * accountsPerPage;
                        int endIdx = min(startIdx + accountsPerPage, static_cast<int>(accounts.size()));
                        for (int i = startIdx; i < endIdx; ++i) {
                            if (accountBlocks[i].getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                                showAccountWindow(
                                    userName,
                                    accounts[i]["account_id"].get<string>(),
                                    accounts[i]["account_name"].get<string>(),
                                    accounts[i]["currency"].get<string>()
                                );
                            }
                        }
                    }
                }
            }
        }

        window.clear(sf::Color::Black);
        window.draw(createAccountButton);
        window.draw(createAccountText);
        window.draw(signOutButton);
        window.draw(signOutText);
        window.draw(welcomeText);

        int startIdx = currentPage * accountsPerPage;
        int endIdx = min(startIdx + accountsPerPage, static_cast<int>(accounts.size()));
        for (int i = startIdx; i < endIdx; ++i) {
            accountBlocks[i].setPosition(300, 50 + (i - startIdx) * 90);
            accountBlocks[i].setFillColor(sf::Color::White);

            sf::Text accountIdText("ID: " + accounts[i]["account_id"].get<string>(), font, 20);
            accountIdText.setPosition(310, 60 + (i - startIdx) * 90);
            accountIdText.setFillColor(sf::Color::Black);

            sf::Text accountNameText("Name: " + accounts[i]["account_name"].get<string>(), font, 20);
            accountNameText.setPosition(310, 80 + (i - startIdx) * 90);
            accountNameText.setFillColor(sf::Color::Black);

            sf::Text accountCurrencyText("Currency: " + accounts[i]["currency"].get<string>(), font, 20);
            accountCurrencyText.setPosition(310, 100 + (i - startIdx) * 90);
            accountCurrencyText.setFillColor(sf::Color::Black);

            window.draw(accountBlocks[i]);
            window.draw(accountIdText);
            window.draw(accountNameText);
            window.draw(accountCurrencyText);
        }

        if (currentPage < totalPages - 1) {
            window.draw(nextButton);
            window.draw(nextText);
        }
        if (currentPage > 0) {
            window.draw(prevButton);
            window.draw(prevText);
        }

        window.display();
    }
}
