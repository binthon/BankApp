#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <ctime>
#include "Header.h"
#include <map>
#include <algorithm> 

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

string getCurrentDateTime() {
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &ltm);
    return string(buffer);
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

    string currentDateTime = getCurrentDateTime();

    for (auto& user : data["users"]) {
        if (user["name"] == userName) {
            if (user.contains("accounts")) {
                for (auto& account : user["accounts"]) {
                    if (account["account_id"] == accountId) {
                        if (!account.contains("in")) {
                            account["in"] = json::array();
                        }
                        if (!account.contains("out")) {
                            account["out"] = json::array();
                        }

                        double currentBalance = stod(account["balance"].get<string>());
                        if (isPay) {
                            currentBalance += amount;
                            account["in"].push_back({ {"amount", amount}, {"date", currentDateTime} });
                        }
                        else {
                            if (currentBalance < amount) {
                                cout << "Insufficient balance for payout." << endl;
                                return;
                            }
                            currentBalance -= amount;
                            account["out"].push_back({ {"amount", amount}, {"date", currentDateTime} });
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
void showHistoryWindow(const string& userName, const string& accountId) {
    sf::RenderWindow historyWindow(sf::VideoMode(1920, 1080), "Transaction History", sf::Style::Titlebar | sf::Style::Close);
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
    logoSprite.setPosition(historyWindow.getSize().x / 2 - logoSprite.getLocalBounds().width / 2, -180);

    sf::Text titleText("Transaction History", font, 30);
    titleText.setFillColor(sf::Color::White);
    sf::FloatRect titleTextBounds = titleText.getLocalBounds();
    titleText.setPosition(historyWindow.getSize().x / 2 - titleTextBounds.width / 2, 250);

    json data;
    try {
        data = loadJson("loginData.json");
    }
    catch (const json::parse_error& e) {
        cout << "JSON parse error: " << e.what() << endl;
        return;
    }

    map<string, pair<float, float>> transactions;
    bool accountFound = false;
    for (const auto& user : data["users"]) {
        if (user["name"] == userName) {
            if (user.contains("accounts")) {
                for (const auto& account : user["accounts"]) {
                    if (account["account_id"] == accountId) {
                        if (account.contains("in")) {
                            for (const auto& transaction : account["in"]) {
                                if (transaction.contains("date") && transaction.contains("amount") && transaction["amount"].is_number()) {
                                    string date = transaction["date"].get<string>().substr(0, 10); 
                                    transactions[date].first += transaction["amount"].get<float>();
                                }
                                else {
                                    cout << "Invalid 'in' transaction format." << endl;
                                }
                            }
                        }
                        if (account.contains("out")) {
                            for (const auto& transaction : account["out"]) {
                                if (transaction.contains("date") && transaction.contains("amount") && transaction["amount"].is_number()) {
                                    string date = transaction["date"].get<string>().substr(0, 10); 
                                    transactions[date].second += transaction["amount"].get<float>();
                                }
                                else {
                                    cout << "Invalid 'out' transaction format." << endl;
                                }
                            }
                        }
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

    float maxTransactionValue = 0.0f;
    for (const auto& entry : transactions) {
        maxTransactionValue = max(maxTransactionValue, entry.second.first + entry.second.second);
    }

    while (historyWindow.isOpen()) {
        sf::Event event;
        while (historyWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                historyWindow.close();
            }
        }

        historyWindow.clear(sf::Color(0x38, 0xB6, 0xFF));
        historyWindow.draw(logoSprite);
        historyWindow.draw(titleText);

        float barWidth = 50.0f;
        float barSpacing = 100.0f;
        float maxHeight = 400.0f;
        float xOffset = 150.0f;
        float yOffset = 800.0f;

        int i = 0;
        for (const auto& entry : transactions) {
            string date = entry.first;
            float inAmount = entry.second.first;
            float outAmount = entry.second.second;

            float inBarHeight = inAmount / maxTransactionValue * maxHeight;
            float outBarHeight = outAmount / maxTransactionValue * maxHeight;

            sf::RectangleShape inBar(sf::Vector2f(barWidth, inBarHeight));
            inBar.setPosition(xOffset + i * (barWidth + barSpacing), yOffset - inBarHeight - outBarHeight);
            inBar.setFillColor(sf::Color::Green);

            sf::RectangleShape outBar(sf::Vector2f(barWidth, outBarHeight));
            outBar.setPosition(xOffset + i * (barWidth + barSpacing), yOffset - outBarHeight);
            outBar.setFillColor(sf::Color::Red);

            sf::Text dateText(date, font, 20);
            sf::FloatRect dateTextBounds = dateText.getLocalBounds();
            dateText.setPosition(xOffset + i * (barWidth + barSpacing) + (barWidth - dateTextBounds.width) / 2, yOffset + 20);
            dateText.setFillColor(sf::Color::White);

            sf::Text inAmountLabel("In: " + to_string(inAmount), font, 20);
            sf::FloatRect inAmountLabelBounds = inAmountLabel.getLocalBounds();
            inAmountLabel.setPosition(xOffset + i * (barWidth + barSpacing) + (barWidth - inAmountLabelBounds.width) / 2, yOffset - inBarHeight - outBarHeight - 50);
            inAmountLabel.setFillColor(sf::Color::White);

            sf::Text outAmountLabel("Out: " + to_string(outAmount), font, 20);
            sf::FloatRect outAmountLabelBounds = outAmountLabel.getLocalBounds();
            outAmountLabel.setPosition(xOffset + i * (barWidth + barSpacing) + (barWidth - outAmountLabelBounds.width) / 2, yOffset - inBarHeight - outBarHeight - 30);
            outAmountLabel.setFillColor(sf::Color::White);

            historyWindow.draw(inBar);
            historyWindow.draw(outBar);
            historyWindow.draw(dateText);
            historyWindow.draw(inAmountLabel);
            historyWindow.draw(outAmountLabel);

            i++;
        }

        historyWindow.display();
    }
}





void showAccountDetailsWindow(const string& userName, const string& accountId, const string& accountName, const string& currency) {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Account Details", sf::Style::Titlebar | sf::Style::Close);
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

    sf::Text welcomeText("Welcome, " + userName, font, 30);
    welcomeText.setPosition(1470, 60);
    welcomeText.setFillColor(sf::Color::White);

    sf::Texture buttonTexture;
    if (!buttonTexture.loadFromFile("save_button.png")) {
        cerr << "Failed to load button texture\n";
        return;
    }

    sf::Texture backbuttonTexture;
    if (!backbuttonTexture.loadFromFile("back_button.png")) {
        cerr << "Failed to load button texture\n";
        return;
    }

    sf::Texture frameTexture;
    if (!frameTexture.loadFromFile("frame.png")) {
        cerr << "Failed to load frame texture\n";
        return;
    }

    sf::Sprite signOutButton(backbuttonTexture);
    signOutButton.setPosition(1750, 50);

    sf::Text signOutText("Sign Out", font, 20);
    signOutText.setPosition(signOutButton.getPosition().x + 45, signOutButton.getPosition().y + 12);
    signOutText.setFillColor(sf::Color::White);

    sf::Text accountIdLabel("Account ID: " + accountId, font, 25);
    accountIdLabel.setPosition(window.getSize().x / 2 - 150, 300);
    accountIdLabel.setFillColor(sf::Color::White);

    sf::Text accountNameLabel("Account Name: " + accountName, font, 25);
    accountNameLabel.setPosition(window.getSize().x / 2 - 150, 350);
    accountNameLabel.setFillColor(sf::Color::White);

    sf::Text currencyLabel("Currency: " + currency, font, 25);
    currencyLabel.setPosition(window.getSize().x / 2 - 150, 400);
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

    sf::Text balanceLabel("Balance: " + balance, font, 25);
    balanceLabel.setPosition(window.getSize().x / 2 - 150, 450);
    balanceLabel.setFillColor(sf::Color::White);

    sf::Sprite amountBox(frameTexture);
    amountBox.setPosition(window.getSize().x / 2 - 75, 500);

    sf::Text amountText("", font, 20);
    amountText.setPosition(amountBox.getPosition().x + 5, amountBox.getPosition().y + 5);
    amountText.setFillColor(sf::Color::Black);

    sf::Sprite payButton(buttonTexture);
    payButton.setPosition(window.getSize().x / 2 - 75, 600);

    sf::Text payText("Pay", font, 20);
    payText.setPosition(payButton.getPosition().x + 80, payButton.getPosition().y + 15);
    payText.setFillColor(sf::Color::White);

    sf::Sprite payoutButton(backbuttonTexture);
    payoutButton.setPosition(window.getSize().x / 2 - 75, 670);

    sf::Text payoutText("Payout", font, 20);
    payoutText.setPosition(payoutButton.getPosition().x + 60, payoutButton.getPosition().y + 15);
    payoutText.setFillColor(sf::Color::White);

    sf::Sprite transferButton(buttonTexture);
    transferButton.setPosition(window.getSize().x / 2 - 75, 740);

    sf::Text transferText("Transfer", font, 20);
    transferText.setPosition(transferButton.getPosition().x + 60, transferButton.getPosition().y + 15);
    transferText.setFillColor(sf::Color::White);

    sf::Sprite depositButton(buttonTexture);
    depositButton.setPosition(window.getSize().x / 2 - 75, 810);

    sf::Text depositText("Deposit", font, 20);
    depositText.setPosition(depositButton.getPosition().x + 60, depositButton.getPosition().y + 15);
    depositText.setFillColor(sf::Color::White);

    sf::Sprite historyButton(buttonTexture);
    historyButton.setPosition(window.getSize().x / 2 - 75, 880);

    sf::Text historyText("History", font, 20);
    historyText.setPosition(historyButton.getPosition().x + 60, historyButton.getPosition().y + 15);
    historyText.setFillColor(sf::Color::White);

    sf::Sprite backButton(backbuttonTexture);
    backButton.setPosition(window.getSize().x / 2 - 75, 950);

    sf::Text backText("Back", font, 20);
    backText.setPosition(backButton.getPosition().x + 80, backButton.getPosition().y + 15);
    backText.setFillColor(sf::Color::White);

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
                    else if (historyButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        showHistoryWindow(userName, accountId);
                    }
                    else if (transferButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        if (hasPin(userName, data)) {
                            window.close();
                            showTransferDetailsWindow(userName, accountId, accountName, balance);
                        }
                        else {
                            errorText.setString("You need to set a PIN to make a transfer.");
                        }
                    }
                    else if (depositButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        window.close();
                        showDepositWindow(userName, accountId, accountName, stod(balance), currency); 
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

        window.clear(sf::Color(0x38, 0xB6, 0xFF));
        window.draw(logoSprite);
        window.draw(welcomeText);
        window.draw(signOutButton);
        window.draw(signOutText);
        window.draw(accountIdLabel);
        window.draw(accountNameLabel);
        window.draw(currencyLabel);
        window.draw(balanceLabel);
        window.draw(amountBox);
        window.draw(amountText);
        window.draw(payButton);
        window.draw(payText);
        window.draw(payoutButton);
        window.draw(payoutText);
        window.draw(transferButton);
        window.draw(transferText);
        window.draw(depositButton);
        window.draw(depositText);
        window.draw(historyButton);
        window.draw(historyText);
        window.draw(backButton);
        window.draw(backText);
        window.draw(errorText);
        window.display();
    }
}
