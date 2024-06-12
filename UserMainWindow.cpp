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
                user["accounts"] = json::array();
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
void showStatsWindow(const string& userName, const vector<json>& accounts) {
    sf::RenderWindow statsWindow(sf::VideoMode(1200, 700), "Account Statistics", sf::Style::Titlebar | sf::Style::Close); // Increased window size
    sf::Font font;

    if (!font.loadFromFile("TTFors.ttf")) {
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

        statsWindow.clear(sf::Color(30, 30, 30));

        float barWidth = 50.0f;
        float barSpacing = 100.0f;
        float maxHeight = 300.0f;
        float xOffset = 100.0f;
        float yOffset = 500.0f;

        // Title text
        sf::Text title("Account Statistics", font, 30);
        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setPosition((statsWindow.getSize().x - titleBounds.width) / 2, 20);
        title.setFillColor(sf::Color::White);
        statsWindow.draw(title);

        
        sf::Text subtitle("Income and Expenses", font, 20);
        sf::FloatRect subtitleBounds = subtitle.getLocalBounds();
        subtitle.setPosition((statsWindow.getSize().x - subtitleBounds.width) / 2, 60);
        subtitle.setFillColor(sf::Color::White);
        statsWindow.draw(subtitle);

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

          
            std::ostringstream inStream, outStream;
            inStream << std::fixed << std::setprecision(2) << inAmount << " (" << (inAmount / totalAmount * 100.0f) << "%)";
            outStream << std::fixed << std::setprecision(2) << outAmount << " (" << (outAmount / totalAmount * 100.0f) << "%)";

            sf::Text inText("In: " + inStream.str(), font, 15);
            sf::FloatRect inTextBounds = inText.getLocalBounds();
            inText.setPosition(xOffset + i * (barWidth + barSpacing) + (barWidth - inTextBounds.width) / 2, yOffset - inHeight - outHeight - 60);
            inText.setFillColor(sf::Color::White);

            sf::Text outText("Out: " + outStream.str(), font, 15);
            sf::FloatRect outTextBounds = outText.getLocalBounds();
            outText.setPosition(xOffset + i * (barWidth + barSpacing) + (barWidth - outTextBounds.width) / 2, yOffset - inHeight - outHeight - 40);
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
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "User Main Window", sf::Style::Titlebar | sf::Style::Close);
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

    sf::Texture test;
    if (!test.loadFromFile("text.png")) {
        cerr << "Failed to load button texture\n";
        return;
    }
    sf::Texture backbuttonTexture;
    if (!backbuttonTexture.loadFromFile("back_button.png")) {
        cerr << "Failed to load button texture\n";
        return;
    }

    sf::Texture stat;
    if (!stat.loadFromFile("stat.png")) {
        cerr << "Failed to load button texture\n";
        return;
    }
    sf::Sprite signOutButton(backbuttonTexture);
    signOutButton.setPosition(1750, 50);

    sf::Sprite createAccountButton(buttonTexture);
    createAccountButton.setPosition(window.getSize().x / 2 - 100, 880);

    sf::Text createAccountText("Create Account", font, 20);
    createAccountText.setPosition(createAccountButton.getPosition().x + 20, createAccountButton.getPosition().y + 15);
    createAccountText.setFillColor(sf::Color::White);

    sf::Sprite statsButton(stat);
    statsButton.setPosition(window.getSize().x / 2 - 100, 200);

    sf::Text statsButtonText("Stats", font, 20);
    statsButtonText.setPosition(statsButton.getPosition().x + 60, statsButton.getPosition().y + 15);
    statsButtonText.setFillColor(sf::Color::White);

    sf::Text signOutText("Sign Out", font, 20);
    signOutText.setPosition(signOutButton.getPosition().x + 45, signOutButton.getPosition().y + 12);
    signOutText.setFillColor(sf::Color::White);

    sf::Text welcomeText("Welcome, " + userName, font, 30);
    welcomeText.setPosition(1470, 60);
    welcomeText.setFillColor(sf::Color::White);

    sf::Text pinLabel("Set PIN:", font, 20);
    pinLabel.setPosition(100, 300);
    pinLabel.setFillColor(sf::Color::White);

    sf::Sprite pinInputBox(frameTexture);
    pinInputBox.setPosition(100, 330);

    sf::Text pinInputText("", font, 20);
    pinInputText.setPosition(110, 335);
    pinInputText.setFillColor(sf::Color::Black);

    sf::Text confirmPinLabel("Old PIN:", font, 20);
    confirmPinLabel.setPosition(100, 380);
    confirmPinLabel.setFillColor(sf::Color::White);

    sf::Sprite confirmPinInputBox(frameTexture);
    confirmPinInputBox.setPosition(100, 410);

    sf::Text confirmPinInputText("", font, 20);
    confirmPinInputText.setPosition(110, 415);
    confirmPinInputText.setFillColor(sf::Color::Black);

    sf::Sprite savePinButton(buttonTexture);
    savePinButton.setPosition(100, 470);

    sf::Text savePinText("Save PIN", font, 20);
    savePinText.setPosition(savePinButton.getPosition().x + 10, savePinButton.getPosition().y + 50);
    savePinText.setFillColor(sf::Color::White);


    json data = loadJsonData("loginData.json");
    vector<json> accounts = getUserAccounts(userName, data);

    int currentPage = 0;
    const int accountsPerPage = 4;
    int totalPages = (accounts.size() + accountsPerPage - 1) / accountsPerPage;

    sf::Sprite nextButton(buttonTexture);
    nextButton.setPosition(1010, 780);

    sf::Text nextText(">", font, 30);
    nextText.setPosition(nextButton.getPosition().x + 90, nextButton.getPosition().y + 10);
    nextText.setFillColor(sf::Color::White);

    sf::Sprite prevButton(buttonTexture);
    prevButton.setPosition(720, 780);

    sf::Text prevText("<", font, 30);
    prevText.setPosition(prevButton.getPosition().x + 80, prevButton.getPosition().y + 10);
    prevText.setFillColor(sf::Color::White);

    vector<sf::Sprite> accountBlocks(accountsPerPage, sf::Sprite(test));
    vector<sf::Sprite> deleteButtons(accountsPerPage, sf::Sprite(backbuttonTexture));

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
                if (event.text.unicode == 8) { 
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
                                    if (!isPinSet || user["pin"] == confirmPinInput) {
                                        user["pin"] = pinInput;
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

        window.clear(sf::Color(0x38, 0xB6, 0xFF));

        window.draw(logoSprite);
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
            savePinButton.setPosition(100, 470); 
            savePinText.setPosition(savePinButton.getPosition().x + 40, savePinButton.getPosition().y + 15);
        }
        else {
            savePinButton.setPosition(100, 380); 
            savePinText.setPosition(savePinButton.getPosition().x + 40, savePinButton.getPosition().y + 15);
        }
        window.draw(savePinButton);
        window.draw(savePinText);
        window.draw(statsButton);
        window.draw(statsButtonText);

        int startIdx = currentPage * accountsPerPage;
        int endIdx = min(startIdx + accountsPerPage, static_cast<int>(accounts.size()));
        for (int i = startIdx; i < endIdx; ++i) {
            accountBlocks[i - startIdx].setPosition(window.getSize().x / 2 - accountBlocks[i - startIdx].getGlobalBounds().width / 2, 300 + (i - startIdx) * 120);

            deleteButtons[i - startIdx].setPosition(accountBlocks[i - startIdx].getPosition().x + accountBlocks[i - startIdx].getGlobalBounds().width + 10, accountBlocks[i - startIdx].getPosition().y + 20);

            sf::Text accountIdText("ID: " + accounts[i]["account_id"].get<string>(), font, 20);
            accountIdText.setPosition(accountBlocks[i - startIdx].getPosition().x + 30, accountBlocks[i - startIdx].getPosition().y + 20);
            accountIdText.setFillColor(sf::Color::White);

            sf::Text accountNameText("Name: " + accounts[i]["account_name"].get<string>(), font, 20);
            accountNameText.setPosition(accountBlocks[i - startIdx].getPosition().x + 30, accountBlocks[i - startIdx].getPosition().y + 40);
            accountNameText.setFillColor(sf::Color::White);

            sf::Text accountCurrencyText("Currency: " + accounts[i]["currency"].get<string>(), font, 20);
            accountCurrencyText.setPosition(accountBlocks[i - startIdx].getPosition().x + 30, accountBlocks[i - startIdx].getPosition().y + 60);
            accountCurrencyText.setFillColor(sf::Color::White);

            sf::Text deleteButtonText("Delete", font, 20);
            deleteButtonText.setPosition(deleteButtons[i - startIdx].getPosition().x + 65, deleteButtons[i - startIdx].getPosition().y+15);
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
