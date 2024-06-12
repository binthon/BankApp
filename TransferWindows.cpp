#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <ctime>
#include <curl/curl.h>
#include "Header.h"

using namespace std;
using json = nlohmann::json;

json loadJsonTransfer(const std::string& filePath) {
    ifstream inputFile(filePath);
    json data;

    if (inputFile.good()) {
        try {
            string fileContent((std::istreambuf_iterator<char>(inputFile)),
                istreambuf_iterator<char>());
            cout << "Plik JSON zawiera: " << fileContent << endl;
            data = json::parse(fileContent);
        }
        catch (json::parse_error& e) {
            cerr << "JSON parse error: " << e.what() << endl;
            cerr << "Exception id: " << e.id << endl;
            cerr << "Byte position of error: " << e.byte << endl;
        }
    }
    else {
        cout << "Failed to open file: " << filePath << endl;
    }

    return data;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

double getExchangeRate(const string& apiKey, const string& fromCurrency, const string& toCurrency) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    string url = "https://v6.exchangerate-api.com/v6/" + apiKey + "/pair/" + fromCurrency + "/" + toCurrency;

    cout << "Wywo³anie API:\n";
    cout << "URL: " << url << endl;
    cout << "API Key: " << apiKey << endl;
    cout << "From Currency: " << fromCurrency << endl;
    cout << "To Currency: " << toCurrency << endl;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    auto jsonResponse = json::parse(readBuffer);
    return jsonResponse["conversion_rate"];
}

string getAccountCurrency(const json& data, const string& accountId) {
    for (const auto& user : data["users"]) {
        if (user.contains("accounts")) {
            for (const auto& account : user["accounts"]) {
                if (account["account_id"] == accountId && account.contains("currency")) {
                    return account["currency"];
                }
            }
        }
    }
    return "PLN";
}

int getNextTransferId(const json& data) {
    int maxId = 0;
    for (const auto& user : data["users"]) {
        if (user.contains("accounts")) {
            for (const auto& account : user["accounts"]) {
                if (account.contains("in")) {
                    for (const auto& entry : account["in"]) {
                        if (entry.contains("id") && entry["id"].is_number_integer()) {
                            maxId = max(maxId, entry["id"].get<int>());
                        }
                    }
                }
                if (account.contains("out")) {
                    for (const auto& entry : account["out"]) {
                        if (entry.contains("id") && entry["id"].is_number_integer()) {
                            maxId = max(maxId, entry["id"].get<int>());
                        }
                    }
                }
            }
        }
    }
    return maxId + 1;
}

bool validateTransferPin(const string& userName, const string& enteredPin, const json& data) {
    for (const auto& user : data["users"]) {
        if (user["name"] == userName && user.contains("pin")) {
            return user["pin"] == enteredPin;
        }
    }
    return false;
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

string currentDateTime() {
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &ltm);
    return string(buffer);
}

void updateTransferBalanceInJson(const string& accountId, double amount, bool isPay, int transferId, double exchangeRate, const string& fromAccountId, const string& toAccountId) {
    json data = loadJsonTransfer("loginData.json");

    for (auto& user : data["users"]) {
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
                        account["in"].push_back({
                            {"id", transferId},
                            {"amount", amount},
                            {"exchange_rate", exchangeRate},
                            {"from_account_id", fromAccountId},
                            {"to_account_id", toAccountId},
                            {"date", currentDateTime()}
                            });
                    }
                    else {
                        if (currentBalance < amount) {
                            cout << "Insufficient balance for payout." << endl;
                            return;
                        }
                        currentBalance -= amount;
                        account["out"].push_back({
                            {"id", transferId},
                            {"amount", amount},
                            {"exchange_rate", exchangeRate},
                            {"from_account_id", fromAccountId},
                            {"to_account_id", toAccountId},
                            {"date", currentDateTime()}
                            });
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

void showTransferDetailsWindow(const string& userName, const string& accountId, const string& accountName, string balance) {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Transfer Details", sf::Style::Titlebar | sf::Style::Close);
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

    sf::Texture buttonTexture;
    if (!buttonTexture.loadFromFile("save_button.png")) {
        cerr << "Failed to load button texture\n";
        return;
    }

    sf::Texture backButtonTexture;
    if (!backButtonTexture.loadFromFile("back_button.png")) {
        cerr << "Failed to load button texture\n";
        return;
    }

    sf::Sprite signOutButton(backButtonTexture);
    signOutButton.setPosition(1750, 50);

    sf::Sprite confirmButton(buttonTexture);
    confirmButton.setPosition(window.getSize().x / 2 - 100, 700);

    sf::Sprite backButton(backButtonTexture);
    backButton.setPosition(window.getSize().x / 2 - 100, 770);

    sf::Text signOutText("Sign Out", font, 20);
    signOutText.setPosition(signOutButton.getPosition().x + 45, signOutButton.getPosition().y + 12);
    signOutText.setFillColor(sf::Color::White);

    sf::Text confirmButtonText("Confirm", font, 20);
    confirmButtonText.setPosition(confirmButton.getPosition().x + 55, confirmButton.getPosition().y + 15);
    confirmButtonText.setFillColor(sf::Color::White);

    sf::Text backText("Back", font, 20);
    backText.setPosition(backButton.getPosition().x + 70, backButton.getPosition().y + 15);
    backText.setFillColor(sf::Color::White);

    sf::Text welcomeText("Welcome, " + userName, font, 30);
    welcomeText.setPosition(1470, 60);
    welcomeText.setFillColor(sf::Color::White);

    sf::Text balanceText("Balance: " + balance, font, 25);
    balanceText.setPosition(window.getSize().x / 2 - 150, 220);
    balanceText.setFillColor(sf::Color::White);

    sf::Text infoText("Transfer from Account: " + accountName, font, 25);
    infoText.setPosition(window.getSize().x / 2 - 150, 280);
    infoText.setFillColor(sf::Color::White);

    sf::Text setValueLabel("Set Value:", font, 25);
    setValueLabel.setPosition(window.getSize().x / 2 - 150, 340);
    setValueLabel.setFillColor(sf::Color::White);

    sf::RectangleShape valueBox(sf::Vector2f(300.f, 50.f));
    valueBox.setPosition(window.getSize().x / 2 - 150, 380);
    valueBox.setFillColor(sf::Color::White);
    valueBox.setOutlineColor(sf::Color::Black);
    valueBox.setOutlineThickness(2.f);

    sf::Text valueText("", font, 25);
    valueText.setPosition(valueBox.getPosition().x + 10, valueBox.getPosition().y + 10);
    valueText.setFillColor(sf::Color::Black);

    sf::Text toAccountLabel("To Account ID:", font, 25);
    toAccountLabel.setPosition(window.getSize().x / 2 - 150, 450);
    toAccountLabel.setFillColor(sf::Color::White);

    sf::RectangleShape toAccountBox(sf::Vector2f(400.f, 50.f));
    toAccountBox.setPosition(window.getSize().x / 2 - 150, 490);
    toAccountBox.setFillColor(sf::Color::White);
    toAccountBox.setOutlineColor(sf::Color::Black);
    toAccountBox.setOutlineThickness(2.f);

    sf::Text toAccountText("", font, 25);
    toAccountText.setPosition(toAccountBox.getPosition().x + 10, toAccountBox.getPosition().y + 10);
    toAccountText.setFillColor(sf::Color::Black);

    sf::Text pinLabel("PIN:", font, 25);
    pinLabel.setPosition(window.getSize().x / 2 - 150, 560);
    pinLabel.setFillColor(sf::Color::White);

    sf::RectangleShape pinBox(sf::Vector2f(300.f, 50.f));
    pinBox.setPosition(window.getSize().x / 2 - 150, 600);
    pinBox.setFillColor(sf::Color::White);
    pinBox.setOutlineColor(sf::Color::Black);
    pinBox.setOutlineThickness(2.f);

    sf::Text pinText("", font, 25);
    pinText.setPosition(pinBox.getPosition().x + 10, pinBox.getPosition().y + 10);
    pinText.setFillColor(sf::Color::Black);

    sf::Text pinAsteriskText("", font, 25);
    pinAsteriskText.setPosition(pinBox.getPosition().x + 10, pinBox.getPosition().y + 10);
    pinAsteriskText.setFillColor(sf::Color::Black);

    sf::Text choiceAccountText("Choose Account:", font, 25);
    choiceAccountText.setPosition(1400, 220);
    choiceAccountText.setFillColor(sf::Color::White);

    json data = loadJsonTransfer("loginData.json");
    vector<string> accountNumbers = getTransferAccountNumbers(data, accountId);

    vector<sf::Text> accountTexts;
    float yPos = 260;
    for (const auto& accountNumber : accountNumbers) {
        sf::Text accountText(accountNumber, font, 20);
        accountText.setPosition(1400, yPos);
        accountText.setFillColor(sf::Color::White);
        accountTexts.push_back(accountText);
        yPos += 40;
    }

    string valueInput;
    string selectedAccount;
    string pinInput;
    string pinAsterisk;

    bool valueActive = false;
    bool pinActive = false;
    string apiKey = "93b2b3ff1c15cd7e50e8f74b";

    sf::Text successText("", font, 20);
    successText.setPosition(window.getSize().x / 2 - 100, 850);
    successText.setFillColor(sf::Color::Green);

    sf::Text errorText("", font, 20);
    errorText.setPosition(window.getSize().x / 2 - 100, 900);
    errorText.setFillColor(sf::Color::Red);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::TextEntered) {
                if (valueActive && isdigit(static_cast<char>(event.text.unicode)) && valueInput.size() < 10) {
                    valueInput += static_cast<char>(event.text.unicode);
                    valueText.setString(valueInput);
                }
                else if (valueActive && event.text.unicode == '\b' && !valueInput.empty()) {
                    valueInput.pop_back();
                    valueText.setString(valueInput);
                }

                if (pinActive && isdigit(static_cast<char>(event.text.unicode)) && pinInput.size() < 4) {
                    pinInput += static_cast<char>(event.text.unicode);
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
                                    string fromCurrency = getAccountCurrency(data, accountId);
                                    string toCurrency = getAccountCurrency(data, selectedAccount);

                                    double convertedAmount = amount;
                                    double exchangeRate = 1.0;
                                    if (fromCurrency != toCurrency) {
                                        exchangeRate = getExchangeRate(apiKey, fromCurrency, toCurrency);
                                        convertedAmount = amount * exchangeRate;
                                    }

                                    int transferId = getNextTransferId(data);

                                    updateTransferBalanceInJson(selectedAccount, convertedAmount, true, transferId, exchangeRate, accountId, selectedAccount);
                                    updateTransferBalanceInJson(accountId, amount, false, transferId, exchangeRate, accountId, selectedAccount);

                                    json updatedData = loadJsonTransfer("loginData.json");
                                    for (const auto& user : updatedData["users"]) {
                                        if (user.contains("accounts")) {
                                            for (const auto& account : user["accounts"]) {
                                                if (account["account_id"] == accountId) {
                                                    balance = account["balance"].get<string>();
                                                    break;
                                                }
                                            }
                                        }
                                    }

                                    balanceText.setString("Balance: " + balance);

                                    valueInput.clear();
                                    selectedAccount.clear();
                                    pinInput.clear();
                                    pinAsterisk.clear();
                                    valueText.setString("");
                                    toAccountText.setString("");
                                    pinAsteriskText.setString("");

                                    successText.setString("Transfer successful!");
                                    errorText.setString("");
                                }
                                else {
                                    errorText.setString("Invalid PIN.");
                                }
                            }
                            else {
                                errorText.setString("Amount exceeds balance.");
                            }
                        }
                        else {
                            errorText.setString("Enter amount, select an account, and enter PIN.");
                        }
                    }
                    else {
                        for (size_t i = 0; i < accountTexts.size(); ++i) {
                            if (accountTexts[i].getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                                selectedAccount = accountNumbers[i];
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

        window.clear(sf::Color(0x38, 0xB6, 0xFF));

        window.draw(logoSprite);
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
        window.draw(successText);
        window.draw(errorText);

        for (size_t i = 0; i < accountTexts.size(); ++i) {
            window.draw(accountTexts[i]);
        }

        window.display();
    }
}
