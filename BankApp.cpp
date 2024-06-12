#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include "Header.h"

using namespace std;
using json = nlohmann::json;

void showLoginWindow();
void showSignUpWindow();
void showUserMainWindows(const string& userName);

string xorEncryptDecrypt(const string& input, const string& key) {
    string output = input;
    size_t keyLength = key.length();
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = input[i] ^ key[i % keyLength];
    }
    return output;
}

int main() {
    showLoginWindow();
    cout << "Hello World";
    return 0;
}

struct TextField {
    sf::Sprite frame;
    sf::Text text;
    string input;
    bool isActive = false;

    TextField(float x, float y, const sf::Font& font, const sf::Texture& texture, unsigned int fontSize = 20) {
        frame.setTexture(texture);
        frame.setPosition(x, y);

        text.setFont(font);
        text.setCharacterSize(fontSize);
        text.setPosition(x + 10, y + 8);
        text.setFillColor(sf::Color::Black);
    }

    void setActive(bool active) {
        isActive = active;
    }

    void handleEvent(sf::Event event) {
        if (event.type == sf::Event::TextEntered && isActive) {
            if (event.text.unicode == '\b' && !input.empty()) {
                input.pop_back();
            }
            else if (event.text.unicode < 128 && event.text.unicode != '\b') {
                input += static_cast<char>(event.text.unicode);
            }
            text.setString(input);
        }
    }
};

void setErrorMessage(sf::Text& errorMessage, const std::string& message, const sf::RenderWindow& window) {
    errorMessage.setString(message);
    sf::FloatRect textRect = errorMessage.getLocalBounds();
    errorMessage.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    errorMessage.setPosition(window.getSize().x / 4.0f, window.getSize().y / 2.0f + 185.0f);
}

void showLoginWindow() {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "FORM");
    sf::Font font;
    if (!font.loadFromFile("TTFors.ttf")) {
        cout << "Could not load font\n";
        return;
    }
    sf::Texture frameTexture;
    if (!frameTexture.loadFromFile("frame.png")) {
        cerr << "Failed to load frame texture\n";
        return;
    }
    sf::Sprite logoSprite;
    sf::Texture logoTexture;
    if (!logoTexture.loadFromFile("logo.png")) {
        cerr << "Failed to load logo texture\n";
        return;
    }
    sf::Texture backButtonTexture;
    if (!backButtonTexture.loadFromFile("back_button.png")) {
        cerr << "Failed to load back button texture\n";
        return;
    }
    sf::Texture saveButtonTexture;
    if (!saveButtonTexture.loadFromFile("save_button.png")) {
        cerr << "Failed to load save button texture\n";
        return;
    }
    sf::Texture rightImageTexture;
    if (!rightImageTexture.loadFromFile("right_image.jpg")) { 
        cerr << "Failed to load right image texture\n";
        return;
    }
    sf::Sprite rightImageSprite;
    rightImageSprite.setTexture(rightImageTexture);
    rightImageSprite.setPosition(window.getSize().x / 2, 0);
    rightImageSprite.setScale(window.getSize().x / 2 / rightImageSprite.getLocalBounds().width,
        window.getSize().y / rightImageSprite.getLocalBounds().height);

    logoSprite.setTexture(logoTexture);
    logoSprite.setPosition(window.getSize().x / 4 - logoSprite.getLocalBounds().width / 2, 0);

    float fieldX = window.getSize().x / 4 - 50.f;
    float fieldY = logoSprite.getPosition().y + logoSprite.getLocalBounds().height - 100.f;
    TextField login(fieldX, fieldY+1, font, frameTexture);
    TextField password(fieldX, fieldY + 51, font, frameTexture);

    sf::Vector2f buttonPos(window.getSize().x / 4 + 50, window.getSize().y - 450);
    sf::Sprite backButton(backButtonTexture);
    backButton.setPosition(buttonPos.x - 150, buttonPos.y - 80);

    sf::Sprite saveButton(saveButtonTexture);
    saveButton.setPosition(buttonPos.x - 150, buttonPos.y);

    sf::Vector2f labelPos(fieldX - 115, fieldY + 7);
    sf::Text loginLabel("Login:", font, 20);
    loginLabel.setPosition(labelPos.x, labelPos.y);
    loginLabel.setFillColor(sf::Color::White);

    sf::Text passwordLabel("Password:", font, 20);
    passwordLabel.setPosition(labelPos.x, labelPos.y + 50);
    passwordLabel.setFillColor(sf::Color::White);

    sf::Text backText("Sign in", font, 20);
    backText.setPosition(backButton.getPosition().x + 65, backButton.getPosition().y + 15);
    backText.setFillColor(sf::Color::White);

    sf::Text saveText("Sign up", font, 20);
    saveText.setPosition(saveButton.getPosition().x + 65, saveButton.getPosition().y + 15);
    saveText.setFillColor(sf::Color::White);

    sf::Text errorMessage("", font, 20);
    errorMessage.setFillColor(sf::Color::Red);

    string passwordDisplay = "";
    string actualPassword = "";

    ifstream inputFile("loginData.json");
    json existingData;
    if (inputFile) {
        try {
            inputFile >> existingData;
            if (existingData.find("users") == existingData.end() || !existingData["users"].is_array()) {
                existingData = json{ {"users", json::array()} };
            }
        }
        catch (json::parse_error& e) {
            existingData = json{ {"users", json::array()} };
            cout << "JSON parse error: " << e.what() << endl;
        }
        inputFile.close();
    }
    else {
        existingData = json{ {"users", json::array()} };
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                errorMessage.setString("");

                if (backButton.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                    bool loginSuccess = false;
                    string userName = "";

                    if (login.input.empty() || password.input.empty()) {
                        setErrorMessage(errorMessage, "Empty login or password", window);
                    }
                    else {
                        string encryptedUserID = xorEncryptDecrypt(login.input, "pinokio");
                        string encryptedPassword = xorEncryptDecrypt(password.input, "bankowa");

                        for (const auto& user : existingData["users"]) {
                            if (user["id_user"] == encryptedUserID && user["password"] == encryptedPassword) {
                                loginSuccess = true;
                                userName = user["name"];
                                break;
                            }
                        }
                        if (loginSuccess) {
                            cout << "Login successful" << endl;
                            window.close();
                            showUserMainWindows(userName);
                        }
                        else {
                            setErrorMessage(errorMessage, "Login failed: invalid login or password", window);
                        }
                        login.input.clear();
                        password.input.clear();
                        passwordDisplay.clear();
                        login.text.setString("");
                        password.text.setString("");
                    }
                }
                else if (saveButton.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                    window.close();
                    showSignUpWindow();
                }

                if (login.frame.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                    login.setActive(true);
                    password.setActive(false);
                }
                else if (password.frame.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                    login.setActive(false);
                    password.setActive(true);
                }
                else {
                    login.setActive(false);
                    password.setActive(false);
                }
            }

            if (event.type == sf::Event::TextEntered) {
                if (login.isActive) {
                    login.handleEvent(event);
                }
                else if (password.isActive) {
                    if (event.text.unicode == '\b' && !actualPassword.empty()) {
                        actualPassword.pop_back();
                        passwordDisplay.pop_back();
                    }
                    else if (event.text.unicode < 128 && event.text.unicode != '\b') {
                        actualPassword += static_cast<char>(event.text.unicode);
                        passwordDisplay += '*';
                    }
                    password.input = actualPassword;
                    password.text.setString(passwordDisplay);
                }
            }
        }

        window.clear(sf::Color(0x38, 0xB6, 0xFF));
        window.draw(logoSprite);
        window.draw(login.frame);
        window.draw(login.text);
        window.draw(password.frame);
        window.draw(password.text);
        window.draw(loginLabel);
        window.draw(passwordLabel);
        window.draw(backButton);
        window.draw(saveButton);
        window.draw(backText);
        window.draw(saveText);
        window.draw(errorMessage);
        window.draw(rightImageSprite); 
        window.display();
    }
}
