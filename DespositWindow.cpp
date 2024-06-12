#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "Header.h"

using namespace std;
using json = nlohmann::json;

string getCurrentDate() {
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &ltm);
    return string(buffer);
}

void highlightButton(sf::RectangleShape& button, sf::Text& text, bool highlight) {
    if (highlight) {
        button.setFillColor(sf::Color::Yellow);
        text.setFillColor(sf::Color::Black);
    }
    else {
        button.setFillColor(sf::Color::White);
        text.setFillColor(sf::Color::Black);
    }
}

void resetButtons(vector<sf::RectangleShape>& buttons, vector<sf::Text>& texts) {
    for (size_t i = 0; i < buttons.size(); ++i) {
        highlightButton(buttons[i], texts[i], false);
    }
}

json jsonLoad(const string& fileName) {
    ifstream inFile(fileName);
    json data;
    if (inFile.is_open()) {
        inFile >> data;
        inFile.close();
    }
    else {
        cout << "Could not open file: " << fileName << endl;
    }
    return data;
}

void updateJsonFile(const string& userName, const string& accountId, double depositAmount, const string& percentage, const string& duration) {
    json data = jsonLoad("loginData.json");

    for (auto& user : data["users"]) {
        if (user["name"] == userName) {
            for (auto& account : user["accounts"]) {
                if (account["account_id"] == accountId) {
                    account["balance"] = to_string(stod(account["balance"].get<string>()) - depositAmount);
                    if (!account.contains("deposits") || !account["deposits"].is_array()) {
                        account["deposits"] = json::array();
                    }
                    string depositId = to_string(time(0));
                    account["deposits"].push_back({
                        {"id", depositId},
                        {"amount", depositAmount},
                        {"date", getCurrentDate()},
                        {"percentage", percentage},
                        {"duration", duration}
                        });
                    break;
                }
            }
            break;
        }
    }

    ofstream outFile("loginData.json");
    if (outFile.is_open()) {
        outFile << data.dump(4);
        outFile.close();
    }
    else {
        cout << "Could not open file for writing." << endl;
    }
}

void drawDepositInfo(sf::RenderWindow& window, sf::Font& font, const vector<json>& deposits, int currentPage, const string& currency, vector<sf::FloatRect>& depositRects, vector<sf::FloatRect>& deleteRects) {
    int startIndex = currentPage * 3;
    for (int i = 0; i < 3; ++i) {
        int depositIndex = startIndex + i;
        if (depositIndex >= deposits.size()) {
            break;
        }
        const auto& deposit = deposits[depositIndex];
        double amount = deposit["amount"];
        string percentage = deposit["percentage"];
        string duration = deposit["duration"];
        double earnings = amount * stod(percentage.substr(0, percentage.size() - 1)) / 100 * (stod(duration.substr(0, duration.find(' '))) / 12.0);
        string depositTextStr = "Deposit: " + to_string(amount) + " " + currency + ", " + percentage + ", " + duration + ", Earnings: " + to_string(earnings);

        sf::Text depositInfo(depositTextStr, font, 20);
        depositInfo.setPosition(10, 80 + 60 * i);
        depositInfo.setFillColor(sf::Color::White);

        sf::FloatRect textBounds = depositInfo.getGlobalBounds();
        sf::RectangleShape line(sf::Vector2f(textBounds.width, 2));
        line.setPosition(10, 120 + 60 * i);
        line.setFillColor(sf::Color::White);


        sf::RectangleShape deleteButton(sf::Vector2f(20.f, 20.f));
        deleteButton.setPosition(textBounds.left + textBounds.width + 10, textBounds.top + 5);
        deleteButton.setFillColor(sf::Color::Red);

        window.draw(depositInfo);
        window.draw(line);
        window.draw(deleteButton);

        depositRects.push_back(textBounds);
        deleteRects.push_back(deleteButton.getGlobalBounds());
    }
}

void showDepositWindow(const string& userName, const string& accountId, const string& accountName, double balance, const string& currency) {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Deposit Window", sf::Style::Titlebar | sf::Style::Close);
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

    sf::Text signOutText("Sign Out", font, 20);
    signOutText.setPosition(signOutButton.getPosition().x + 45, signOutButton.getPosition().y + 12);
    signOutText.setFillColor(sf::Color::White);

    sf::Sprite backButton(backButtonTexture);
    backButton.setPosition(window.getSize().x / 2 - 100, 700);

  
    sf::Text backText("Back", font, 20);
    backText.setPosition(backButton.getPosition().x + 65, backButton.getPosition().y + 15);
    backText.setFillColor(sf::Color::White);

    sf::Text welcomeText("Welcome, " + userName, font, 30);
    welcomeText.setPosition(1470, 60);
    welcomeText.setFillColor(sf::Color::White);

    sf::Text balanceText("Balance: " + to_string(balance) + " " + currency, font, 30);
    sf::FloatRect balanceTextBounds = balanceText.getLocalBounds();
    balanceText.setPosition(window.getSize().x / 2 - balanceTextBounds.width / 2, 200);
    balanceText.setFillColor(sf::Color::White);

    vector<string> percentages = { "3%", "4%", "5%" };
    vector<string> months = { "3 months", "5 months", "6 months" };

    vector<sf::RectangleShape> percentButtons(3);
    vector<sf::Text> percentTexts(3);

    vector<sf::RectangleShape> monthButtons(3);
    vector<sf::Text> monthTexts(3);

    string selectedPercentage = "3%";
    string selectedDuration = "3 months";
    double potentialEarnings = 0.0;

    for (int i = 0; i < 3; ++i) {

        percentButtons[i].setSize(sf::Vector2f(150.f, 50.f));
        percentButtons[i].setPosition(window.getSize().x / 2 - 260 + (160 * i), 300);
        percentButtons[i].setFillColor(sf::Color::White);
        percentButtons[i].setOutlineColor(sf::Color::Black);
        percentButtons[i].setOutlineThickness(2.f);

        percentTexts[i].setFont(font);
        percentTexts[i].setString(percentages[i]);
        percentTexts[i].setCharacterSize(20);
        percentTexts[i].setFillColor(sf::Color::Black);
        percentTexts[i].setPosition(percentButtons[i].getPosition().x + 20, percentButtons[i].getPosition().y + 10);

        monthButtons[i].setSize(sf::Vector2f(150.f, 50.f));
        monthButtons[i].setPosition(window.getSize().x / 2 - 260 + (160 * i), 400);
        monthButtons[i].setFillColor(sf::Color::White);
        monthButtons[i].setOutlineColor(sf::Color::Black);
        monthButtons[i].setOutlineThickness(2.f);

        monthTexts[i].setFont(font);
        monthTexts[i].setString(months[i]);
        monthTexts[i].setCharacterSize(20);
        monthTexts[i].setFillColor(sf::Color::Black);
        monthTexts[i].setPosition(monthButtons[i].getPosition().x + 20, monthButtons[i].getPosition().y + 10);
    }

    sf::RectangleShape depositBox(sf::Vector2f(300.f, 50.f));
    depositBox.setPosition(window.getSize().x / 2 - 150, 500);
    depositBox.setFillColor(sf::Color::White);
    depositBox.setOutlineColor(sf::Color::Black);
    depositBox.setOutlineThickness(2.f);

    sf::Text depositText("", font, 20);
    depositText.setPosition(depositBox.getPosition().x + 10, depositBox.getPosition().y + 10);
    depositText.setFillColor(sf::Color::Black);

    sf::Sprite commitButton(buttonTexture);
    commitButton.setPosition(window.getSize().x / 2 - 100, 600);

    sf::Text commitText("Commit", font, 20);
    commitText.setPosition(commitButton.getPosition().x + 50, commitButton.getPosition().y + 15);
    commitText.setFillColor(sf::Color::White);

    sf::Text errorText("", font, 20);
    errorText.setPosition(window.getSize().x / 2 - 120, 800);
    errorText.setFillColor(sf::Color::Yellow);

    sf::Text potentialEarningsText("", font, 20);
    potentialEarningsText.setPosition(window.getSize().x / 2 - 120, 850);
    potentialEarningsText.setFillColor(sf::Color::White);

    json data = jsonLoad("loginData.json");
    vector<json> deposits;
    for (const auto& user : data["users"]) {
        if (user["name"] == userName) {
            for (const auto& account : user["accounts"]) {
                if (account["account_id"] == accountId) {
                    if (account.contains("deposits") && account["deposits"].is_array()) {
                        deposits = account["deposits"].get<vector<json>>();
                    }
                    break;
                }
            }
            break;
        }
    }

    int currentPage = 0;
    string depositInput;
    vector<sf::FloatRect> depositRects;
    vector<sf::FloatRect> deleteRects;
    int selectedPercentButton = 0;
    int selectedMonthButton = 0;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::TextEntered) {
                if (isdigit(event.text.unicode) && depositInput.size() < 10) {
                    depositInput += event.text.unicode;
                    depositText.setString(depositInput);

                    if (!depositInput.empty()) {
                        double depositAmount = stod(depositInput);
                        double percentage = stod(selectedPercentage.substr(0, selectedPercentage.size() - 1)) / 100;
                        double durationMonths = stod(selectedDuration.substr(0, selectedDuration.find(' ')));
                        potentialEarnings = depositAmount * percentage * (durationMonths / 12.0);
                        potentialEarningsText.setString("Potential Earnings: " + to_string(potentialEarnings));
                    }
                }
                else if (event.text.unicode == '\b' && !depositInput.empty()) {
                    depositInput.pop_back();
                    depositText.setString(depositInput);

                    if (!depositInput.empty()) {
                        double depositAmount = stod(depositInput);
                        double percentage = stod(selectedPercentage.substr(0, selectedPercentage.size() - 1)) / 100;
                        double durationMonths = stod(selectedDuration.substr(0, selectedDuration.find(' ')));
                        potentialEarnings = depositAmount * percentage * (durationMonths / 12.0);
                        potentialEarningsText.setString("Potential Earnings: " + to_string(potentialEarnings));
                    }
                    else {
                        potentialEarningsText.setString("");
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                    if (backButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        window.close();
                        showAccountDetailsWindow(userName, accountId, accountName, currency);
                    }
                    else if (signOutButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        window.close();
                        showLoginWindow();
                    }
                    else if (commitButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        if (!depositInput.empty() && stod(depositInput) <= balance) {
                            double depositAmount = stod(depositInput);
                            updateJsonFile(userName, accountId, depositAmount, selectedPercentage, selectedDuration);
                            balance -= depositAmount;
                            balanceText.setString("Balance: " + to_string(balance) + " " + currency);

                            depositInput.clear();
                            depositText.setString("");
                            errorText.setString("Deposit added successfully!");

                            data = jsonLoad("loginData.json");
                            deposits.clear();
                            for (const auto& user : data["users"]) {
                                if (user["name"] == userName) {
                                    for (const auto& account : user["accounts"]) {
                                        if (account["account_id"] == accountId) {
                                            if (account.contains("deposits") && account["deposits"].is_array()) {
                                                deposits = account["deposits"].get<vector<json>>();
                                            }
                                            break;
                                        }
                                    }
                                    break;
                                }
                            }
                            currentPage = 0;
                        }
                        else {
                            errorText.setString("Invalid deposit amount or insufficient balance.");
                        }
                    }
                    else {
                        for (int i = 0; i < 3; ++i) {
                            if (percentButtons[i].getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                                selectedPercentButton = i;
                                selectedPercentage = percentages[i];
                            }
                            if (monthButtons[i].getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                                selectedMonthButton = i;
                                selectedDuration = months[i];
                            }
                        }
                        for (size_t i = 0; i < deleteRects.size(); ++i) {
                            if (deleteRects[i].contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                                double depositAmount = deposits[currentPage * 3 + i]["amount"];
                                balance += depositAmount;
                                balanceText.setString("Balance: " + to_string(balance) + " " + currency);
                                deposits.erase(deposits.begin() + (currentPage * 3 + i));
                                data = jsonLoad("loginData.json");
                                for (auto& user : data["users"]) {
                                    if (user["name"] == userName) {
                                        for (auto& account : user["accounts"]) {
                                            if (account["account_id"] == accountId) {
                                                account["deposits"] = deposits;
                                                account["balance"] = to_string(balance);
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                }
                                ofstream outFile("loginData.json");
                                if (outFile.is_open()) {
                                    outFile << data.dump(4);
                                    outFile.close();
                                }
                                if (deposits.empty()) {
                                    errorText.setString("No deposits available.");
                                }
                                break;
                            }
                        }
                    }
                }
            }
        } 
        window.clear(sf::Color(0x38, 0xB6, 0xFF));

        window.draw(logoSprite);
        window.draw(welcomeText);
        window.draw(balanceText);
        window.draw(signOutButton);
        window.draw(signOutText);
        window.draw(depositBox);
        window.draw(depositText);
        window.draw(commitButton);
        window.draw(commitText);
        window.draw(errorText);
        window.draw(potentialEarningsText);
        window.draw(backButton);
        window.draw(backText);

        depositRects.clear();
        deleteRects.clear();
        drawDepositInfo(window, font, deposits, currentPage, currency, depositRects, deleteRects);

        for (int i = 0; i < 3; ++i) {
            if (i == selectedPercentButton) {
                highlightButton(percentButtons[i], percentTexts[i], true);
            }
            else {
                highlightButton(percentButtons[i], percentTexts[i], false);
            }

            if (i == selectedMonthButton) {
                highlightButton(monthButtons[i], monthTexts[i], true);
            }
            else {
                highlightButton(monthButtons[i], monthTexts[i], false);
            }

            window.draw(percentButtons[i]);
            window.draw(percentTexts[i]);
            window.draw(monthButtons[i]);
            window.draw(monthTexts[i]);
        }

        window.display();
    }
}
