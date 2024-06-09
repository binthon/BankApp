#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <random>
#include "Header.h"
#include <iomanip>
#include <sstream>

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

void saveJsonData(const string& filePath, const json& data) {
    ofstream outputFile(filePath);
    if (outputFile.good()) {
        outputFile << data.dump(4);
    }
}

#include <iomanip>
#include <sstream>

void showStatsWindow(const string& userName, const vector<json>& accounts) {
    sf::RenderWindow statsWindow(sf::VideoMode(1000, 600), "Account Statistics", sf::Style::Titlebar | sf::Style::Close); // Increased window width
    sf::Font font;

    if (!font.loadFromFile("arial.ttf")) {
        cout << "Could not load font\n";
        return;
    }

    vector<pair<string, float>> accountBalances;

    for (const auto& account : accounts) {
        float inAmount = 0.0f;
        float outAmount = 0.0f;

        if (account.contains("in") && account["in"].is_array()) {
            for (const auto& entry : account["in"]) {
                if (entry.contains("amount") && entry["amount"].is_number()) {
                    inAmount += entry["amount"].get<float>();
                }
            }
        }

        if (account.contains("out") && account["out"].is_array()) {
            for (const auto& entry : account["out"]) {
                if (entry.contains("amount") && entry["amount"].is_number()) {
                    outAmount += entry["amount"].get<float>();
                }
            }
        }

        float totalAmount = inAmount + outAmount;
        accountBalances.push_back({ account["account_name"].get<string>(), totalAmount });
    }

    while (statsWindow.isOpen()) {
        sf::Event event;
        while (statsWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                statsWindow.close();
            }
        }

        statsWindow.clear(sf::Color::Black);

        float barWidth = 50.0f;
        float barSpacing = 80.0f; // Increased spacing between bars
        float maxHeight = 300.0f;
        float xOffset = 100.0f;
        float yOffset = 400.0f;

        for (size_t i = 0; i < accountBalances.size(); ++i) {
            const auto& accountBalance = accountBalances[i];
            const string& accountName = accountBalance.first;
            float totalAmount = accountBalance.second;
            float inAmount = 0.0f;
            float outAmount = 0.0f;

            for (const auto& account : accounts) {
                if (account["account_name"].get<string>() == accountName) {
                    if (account.contains("in") && account["in"].is_array()) {
                        for (const auto& entry : account["in"]) {
                            if (entry.contains("amount") && entry["amount"].is_number()) {
                                inAmount += entry["amount"].get<float>();
                            }
                        }
                    }

                    if (account.contains("out") && account["out"].is_array()) {
                        for (const auto& entry : account["out"]) {
                            if (entry.contains("amount") && entry["amount"].is_number()) {
                                outAmount += entry["amount"].get<float>();
                            }
                        }
                    }
                    break;
                }
            }

            float inHeight = (inAmount / totalAmount) * maxHeight;
            float outHeight = (outAmount / totalAmount) * maxHeight;

            sf::RectangleShape inBar(sf::Vector2f(barWidth, inHeight));
            inBar.setPosition(xOffset + i * (barWidth + barSpacing), yOffset - inHeight);
            inBar.setFillColor(sf::Color::Green);

            sf::RectangleShape outBar(sf::Vector2f(barWidth, outHeight));
            outBar.setPosition(xOffset + i * (barWidth + barSpacing), yOffset - inHeight - outHeight);
            outBar.setFillColor(sf::Color::Red);

            // Create labels for in and out amounts with percentages
            std::ostringstream inStream, outStream;
            inStream << std::fixed << std::setprecision(2) << inAmount << " (" << (inAmount / totalAmount * 100.0f) << "%)";
            outStream << std::fixed << std::setprecision(2) << outAmount << " (" << (outAmount / totalAmount * 100.0f) << "%)";

            sf::Text inText("In: " + inStream.str(), font, 15);
            sf::FloatRect inTextBounds = inText.getLocalBounds();
            inText.setPosition(xOffset + i * (barWidth + barSpacing) + (barWidth - inTextBounds.width) / 2, yOffset - inHeight - outHeight - 50);
            inText.setFillColor(sf::Color::White);

            sf::Text outText("Out: " + outStream.str(), font, 15);
            sf::FloatRect outTextBounds = outText.getLocalBounds();
            outText.setPosition(xOffset + i * (barWidth + barSpacing) + (barWidth - outTextBounds.width) / 2, yOffset - inHeight - outHeight - 30);
            outText.setFillColor(sf::Color::White);

            sf::Text accountNameText(accountName, font, 20);
            sf::FloatRect accountNameBounds = accountNameText.getLocalBounds();
            accountNameText.setPosition(xOffset + i * (barWidth + barSpacing) + (barWidth - accountNameBounds.width) / 2, yOffset + 10);
            accountNameText.setFillColor(sf::Color::White);

            statsWindow.draw(inBar);
            statsWindow.draw(outBar);
            statsWindow.draw(accountNameText);
            statsWindow.draw(inText);
            statsWindow.draw(outText);
        }

        statsWindow.display();
    }
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

    sf::RectangleShape signOutButton(sf::Vector2f(150.f, 50.f));
    signOutButton.setPosition(850, 10);
    signOutButton.setFillColor(sf::Color::Red);

    sf::Text signOutText("Sign Out", font, 20);
    signOutText.setPosition(865, 20);
    signOutText.setFillColor(sf::Color::White);

    sf::Text welcomeText("Welcome, " + userName, font, 20);
    welcomeText.setPosition(10, 10);
    welcomeText.setFillColor(sf::Color::White);

    sf::Text pinLabel("Set PIN:", font, 20);
    pinLabel.setPosition(10, 50);
    pinLabel.setFillColor(sf::Color::White);

    sf::RectangleShape pinInputBox(sf::Vector2f(200.f, 40.f));
    pinInputBox.setPosition(10, 80);
    pinInputBox.setFillColor(sf::Color::White);

    sf::Text pinInputText("", font, 20);
    pinInputText.setPosition(20, 85);
    pinInputText.setFillColor(sf::Color::Black);

    sf::Text confirmPinLabel("Old PIN:", font, 20);
    confirmPinLabel.setPosition(10, 130);  // Positioned above the save button
    confirmPinLabel.setFillColor(sf::Color::White);

    sf::RectangleShape confirmPinInputBox(sf::Vector2f(200.f, 40.f));
    confirmPinInputBox.setPosition(10, 160);  // Positioned above the save button
    confirmPinInputBox.setFillColor(sf::Color::White);

    sf::Text confirmPinInputText("", font, 20);
    confirmPinInputText.setPosition(20, 165);
    confirmPinInputText.setFillColor(sf::Color::Black);

    sf::RectangleShape savePinButton(sf::Vector2f(100.f, 40.f));
    savePinButton.setPosition(10, 210);  // Adjusted position to be below the confirm pin input box
    savePinButton.setFillColor(sf::Color::Green);

    sf::Text savePinText("Save PIN", font, 20);
    savePinText.setPosition(20, 215);
    savePinText.setFillColor(sf::Color::White);

    sf::RectangleShape statsButton(sf::Vector2f(100.f, 40.f));
    statsButton.setPosition(10, 270);  // Adjusted position below the save pin button
    statsButton.setFillColor(sf::Color::Blue);

    sf::Text statsButtonText("Stats", font, 20);
    statsButtonText.setPosition(20, 275);
    statsButtonText.setFillColor(sf::Color::White);

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

    vector<sf::RectangleShape> accountBlocks(accountsPerPage, sf::RectangleShape(sf::Vector2f(400.f, 80.f)));
    vector<sf::RectangleShape> deleteButtons(accountsPerPage, sf::RectangleShape(sf::Vector2f(80.f, 40.f)));

    string pinInput;
    string confirmPinInput;
    string pinDisplay;
    string confirmPinDisplay;
    bool isPinSet = false;

    for (const auto& user : data["users"]) {
        if (user["name"] == userName) {
            isPinSet = user.contains("pin");
            break;
        }
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode >= '0' && event.text.unicode <= '9') {
                    if (pinInputBox.getGlobalBounds().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y) && pinInput.size() < 4) {
                        pinInput += static_cast<char>(event.text.unicode);
                        pinDisplay += '*';
                    }
                    else if (isPinSet && confirmPinInputBox.getGlobalBounds().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y) && confirmPinInput.size() < 4) {
                        confirmPinInput += static_cast<char>(event.text.unicode);
                        confirmPinDisplay += '*';
                    }
                }
                if (event.text.unicode == 8) {  // Handle backspace
                    if (pinInputBox.getGlobalBounds().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y) && !pinInput.empty()) {
                        pinInput.pop_back();
                        pinDisplay.pop_back();
                    }
                    else if (isPinSet && confirmPinInputBox.getGlobalBounds().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y) && !confirmPinInput.empty()) {
                        confirmPinInput.pop_back();
                        confirmPinDisplay.pop_back();
                    }
                }
                pinInputText.setString(pinDisplay);
                confirmPinInputText.setString(confirmPinDisplay);
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
                    else if (savePinButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        if (pinInput.size() == 4 && (!isPinSet || confirmPinInput.size() == 4)) {
                            for (auto& user : data["users"]) {
                                if (user["name"] == userName) {
                                    if (!isPinSet || user["pin"] == confirmPinInput) { // Verify the old PIN
                                        user["pin"] = pinInput;  // Set the new PIN
                                        pinInput.clear();
                                        confirmPinInput.clear();
                                        pinDisplay.clear();
                                        confirmPinDisplay.clear();
                                        pinInputText.setString("");
                                        confirmPinInputText.setString("");
                                        isPinSet = true;
                                    }
                                    else {
                                        cout << "Old PIN does not match!" << endl;
                                    }
                                    break;
                                }
                            }
                            saveJsonData("loginData.json", data);
                        }
                        else {
                            cout << "PIN must be 4 digits long and confirmed!" << endl;
                        }
                    }
                    else if (statsButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        showStatsWindow(userName, accounts);
                    }
                    else {
                        int startIdx = currentPage * accountsPerPage;
                        int endIdx = min(startIdx + accountsPerPage, static_cast<int>(accounts.size()));
                        for (int i = startIdx; i < endIdx; ++i) {
                            if (accountBlocks[i - startIdx].getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                                window.close();
                                showAccountDetailsWindow(
                                    userName,
                                    accounts[i]["account_id"].get<string>(),
                                    accounts[i]["account_name"].get<string>(),
                                    accounts[i]["currency"].get<string>()
                                );
                            }
                            else if (deleteButtons[i - startIdx].getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                                // Remove the account from JSON data
                                auto userIt = find_if(data["users"].begin(), data["users"].end(), [&](const json& user) {
                                    return user["name"] == userName;
                                    });

                                if (userIt != data["users"].end()) {
                                    auto& userAccounts = (*userIt)["accounts"];
                                    if (i < userAccounts.size()) {
                                        userAccounts.erase(userAccounts.begin() + i);
                                    }
                                }

                                saveJsonData("loginData.json", data);
                                accounts = getUserAccounts(userName, data);

                                totalPages = (accounts.size() + accountsPerPage - 1) / accountsPerPage;
                                currentPage = min(currentPage, totalPages - 1);

                                if (accounts.empty()) {
                                    currentPage = 0;
                                }

                                endIdx = min(startIdx + accountsPerPage, static_cast<int>(accounts.size()));
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
        window.draw(pinLabel);
        window.draw(pinInputBox);
        window.draw(pinInputText);
        if (isPinSet) {
            window.draw(confirmPinLabel);
            window.draw(confirmPinInputBox);
            window.draw(confirmPinInputText);
            savePinButton.setPosition(10, 210);  // Position below confirm pin input box
            savePinText.setPosition(20, 215);
        }
        else {
            savePinButton.setPosition(10, 130);  // Position below pin input box
            savePinText.setPosition(20, 135);
        }
        window.draw(savePinButton);
        window.draw(savePinText);
        window.draw(statsButton);
        window.draw(statsButtonText);

        int startIdx = currentPage * accountsPerPage;
        int endIdx = min(startIdx + accountsPerPage, static_cast<int>(accounts.size()));
        for (int i = startIdx; i < endIdx; ++i) {
            accountBlocks[i - startIdx].setPosition(300, 50 + (i - startIdx) * 90);
            accountBlocks[i - startIdx].setFillColor(sf::Color::White);

            deleteButtons[i - startIdx].setPosition(720, 60 + (i - startIdx) * 90);
            deleteButtons[i - startIdx].setFillColor(sf::Color::Red);

            sf::Text accountIdText("ID: " + accounts[i]["account_id"].get<string>(), font, 20);
            accountIdText.setPosition(310, 60 + (i - startIdx) * 90);
            accountIdText.setFillColor(sf::Color::Black);

            sf::Text accountNameText("Name: " + accounts[i]["account_name"].get<string>(), font, 20);
            accountNameText.setPosition(310, 80 + (i - startIdx) * 90);
            accountNameText.setFillColor(sf::Color::Black);

            sf::Text accountCurrencyText("Currency: " + accounts[i]["currency"].get<string>(), font, 20);
            accountCurrencyText.setPosition(310, 100 + (i - startIdx) * 90);
            accountCurrencyText.setFillColor(sf::Color::Black);

            sf::Text deleteButtonText("Delete", font, 20);
            deleteButtonText.setPosition(725, 65 + (i - startIdx) * 90);
            deleteButtonText.setFillColor(sf::Color::White);

            window.draw(accountBlocks[i - startIdx]);
            window.draw(accountIdText);
            window.draw(accountNameText);
            window.draw(accountCurrencyText);
            window.draw(deleteButtons[i - startIdx]);
            window.draw(deleteButtonText);
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
