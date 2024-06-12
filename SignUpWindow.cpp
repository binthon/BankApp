#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <random>
#include "Header.h"

using namespace std;
using json = nlohmann::json;

string xorEncryptDecryptPassword(const string& input, const string& key) {
    string output = input;
    size_t keyLength = key.length();
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = input[i] ^ key[i % keyLength];
    }
    return output;
}

string generateRandomPin() {
    random_device random;
    mt19937 generator(random());
    uniform_int_distribution<> distr(0, 9);
    string pin;
    for (int i = 0; i < 8; ++i) {
        pin += to_string(distr(generator));
    }
    return pin;
}

bool isUniqueIdUser(const string& id_user, const json& data) {
    for (const auto& user : data["users"]) {
        if (xorEncryptDecryptPassword(user["id_user"], "pinokio") == id_user) {
            return false;
        }
    }
    return true;
}

string generateUniquePin(const json& data) {
    string pin;
    do {
        pin = generateRandomPin();
    } while (!isUniqueIdUser(pin, data));
    return pin;
}

struct TextField {
    sf::Sprite box;
    sf::Text text;
    string input;
    bool isActive = false;
    bool isEditable = true;

    TextField(float x, float y, const sf::Font& font, const sf::Texture& texture, unsigned int fontSize = 20, bool editable = true)
        : isEditable(editable) {
        box.setTexture(texture);
        box.setPosition(x, y);

        text.setFont(font);
        text.setCharacterSize(fontSize);
        text.setPosition(x + 10, y + 8);
        text.setFillColor(sf::Color::Black);
    }

    void setActive(bool active) {
        isActive = active;
        if (isActive) {
            box.setColor(sf::Color::White);
        }
        else {
            box.setColor(sf::Color::White);
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

    void draw(sf::RenderWindow& window) {
        window.draw(box);
        window.draw(text);
    }
};

struct PasswordField {
    sf::Sprite box;
    sf::Text text;
    string input;
    bool isActive = false;

    PasswordField(float x, float y, const sf::Font& font, const sf::Texture& texture, unsigned int fontSize = 20) {
        box.setTexture(texture);
        box.setPosition(x, y);

        text.setFont(font);
        text.setCharacterSize(fontSize);
        text.setPosition(x + 10, y + 8);
        text.setFillColor(sf::Color::Black);
    }

    void setActive(bool active) {
        isActive = active;
        if (isActive) {
            box.setColor(sf::Color::White);
        }
        else {
            box.setColor(sf::Color::White);
        }
    }

    void handleEvent(sf::Event event) {
        if (event.type == sf::Event::TextEntered) {
            if (isActive) {
                if (event.text.unicode == '\b' && !input.empty()) {
                    input.pop_back();
                }
                else if (event.text.unicode < 128 && event.text.unicode != '\b') {
                    input += static_cast<char>(event.text.unicode);
                }
                text.setString(string(input.length(), '*'));
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(box);
        window.draw(text);
    }
};

void showSignUpWindow() {
    sf::RenderWindow signUpWindow(sf::VideoMode(1920, 1080), "Sign Up", sf::Style::Titlebar | sf::Style::Close);
    sf::Font font;
    if (!font.loadFromFile("TTFors.ttf")) {
        std::cout << "Could not load font\n";
        return;
    }

    sf::Texture rightImageTexture;
    if (!rightImageTexture.loadFromFile("right_image_2.jpg")) {
        cerr << "Failed to load right image texture\n";
        return;
    }
    sf::Sprite rightImageSprite;
    rightImageSprite.setTexture(rightImageTexture);
    rightImageSprite.setPosition(signUpWindow.getSize().x / 2, 0);
    rightImageSprite.setScale(signUpWindow.getSize().x / 2 / rightImageSprite.getLocalBounds().width,
        signUpWindow.getSize().y / rightImageSprite.getLocalBounds().height);

    sf::Texture frameTexture;
    if (!frameTexture.loadFromFile("frame.png")) {
        cerr << "Failed to load frame texture\n";
        return;
    }

    sf::Texture saveTexture;
    if (!saveTexture.loadFromFile("save_button.png")) {
        cerr << "Failed to load button texture\n";
        return;
    }

    sf::Texture backTexture;
    if (!backTexture.loadFromFile("back_button.png")) {
        cerr << "Failed to load button texture\n";
        return;
    }


    sf::Texture logoTexture;
    if (!logoTexture.loadFromFile("logo.png")) {
        cerr << "Failed to load logo texture\n";
        return;
    }
    sf::Sprite logoSprite;
    logoSprite.setTexture(logoTexture);
    logoSprite.setPosition(signUpWindow.getSize().x / 4 - logoSprite.getLocalBounds().width / 2, 0);


    float fieldX = signUpWindow.getSize().x / 4 - 50.f;
    float fieldY = logoSprite.getPosition().y + logoSprite.getLocalBounds().height - 100.f;
    string generatedPin = generateRandomPin();
    TextField nameField(fieldX + 40, fieldY + 1, font, frameTexture);
    TextField surnameField(fieldX + 40, fieldY + 51, font, frameTexture);
    TextField pinField(fieldX + 40, fieldY + 101, font, frameTexture, 20, false);
    PasswordField passwordField(fieldX + 40, fieldY + 151, font, frameTexture, 20);
    PasswordField confirmPasswordField(fieldX + 40, fieldY + 201, font, frameTexture, 20);
    sf::Vector2f buttonPos(signUpWindow.getSize().x / 4 + 50, signUpWindow.getSize().y - 450);

    sf::Sprite backButton(backTexture);
    backButton.setPosition(buttonPos.x - 150, buttonPos.y + 160);

    sf::Sprite saveButton(saveTexture);
    saveButton.setPosition(buttonPos.x - 150, buttonPos.y + 80);

    sf::Text backText("Back", font, 20);
    backText.setPosition(backButton.getPosition().x + 75, backButton.getPosition().y + 15);
    backText.setFillColor(sf::Color::White);

    sf::Text saveText("Save", font, 20);
    saveText.setPosition(saveButton.getPosition().x + 75, saveButton.getPosition().y + 15);
    saveText.setFillColor(sf::Color::White);

    sf::Vector2f labelPos(fieldX - 155, fieldY + 7);
    sf::Text nameLabel("Name:", font, 20);
    nameLabel.setPosition(labelPos.x, labelPos.y);
    nameLabel.setFillColor(sf::Color::White);

    sf::Text surnameLabel("Surname:", font, 20);
    surnameLabel.setPosition(labelPos.x, labelPos.y + 50);
    surnameLabel.setFillColor(sf::Color::White);

    sf::Text pinLabel("ID:", font, 20);
    pinLabel.setPosition(labelPos.x, labelPos.y + 100);
    pinLabel.setFillColor(sf::Color::White);

    sf::Text passwordLabel("Password:", font, 20);
    passwordLabel.setPosition(labelPos.x, labelPos.y + 150);
    passwordLabel.setFillColor(sf::Color::White);

    sf::Text confirmPasswordLabel("Confirm Password:", font, 20);
    confirmPasswordLabel.setPosition(labelPos.x, labelPos.y + 200);
    confirmPasswordLabel.setFillColor(sf::Color::White);

    sf::Text errorText("", font, 20);
    sf::FloatRect textRect = errorText.getLocalBounds();
    errorText.setPosition(signUpWindow.getSize().x / 5.3f, signUpWindow.getSize().y / 2.0f + 345.0f);
    errorText.setFillColor(sf::Color::Red);

    sf::Text successText("", font, 20);
    successText.setPosition(signUpWindow.getSize().x / 5.3f, signUpWindow.getSize().y / 2.0f + 345.0f);
    successText.setFillColor(sf::Color::Green);

    json data;
    string filePath = "loginData.json";
    ifstream inputFile(filePath);
    try {
        if (inputFile.good()) {
            inputFile >> data;
            inputFile.close();
        }
        else {
            data = json{ {"users", json::array()} };
        }
    }
    catch (json::parse_error& e) {
        cout << "JSON parse error: " << e.what() << endl;
        data = json{ {"users", json::array()} };
    }

    generatedPin = generateUniquePin(data);
    pinField.input = generatedPin;
    pinField.text.setString(generatedPin);

    while (signUpWindow.isOpen()) {
        sf::Event event;
        while (signUpWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                signUpWindow.close();
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (nameField.box.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {
                    nameField.setActive(true);
                    surnameField.setActive(false);
                    pinField.setActive(false);
                    passwordField.setActive(false);
                    confirmPasswordField.setActive(false);
                }
                else if (surnameField.box.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {
                    nameField.setActive(false);
                    surnameField.setActive(true);
                    pinField.setActive(false);
                    passwordField.setActive(false);
                    confirmPasswordField.setActive(false);
                }
                else if (pinField.box.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {
                    nameField.setActive(false);
                    surnameField.setActive(false);
                    pinField.setActive(true);
                    passwordField.setActive(false);
                    confirmPasswordField.setActive(false);
                }
                else if (passwordField.box.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {
                    nameField.setActive(false);
                    surnameField.setActive(false);
                    pinField.setActive(false);
                    passwordField.setActive(true);
                    confirmPasswordField.setActive(false);
                }
                else if (confirmPasswordField.box.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {
                    nameField.setActive(false);
                    surnameField.setActive(false);
                    pinField.setActive(false);
                    passwordField.setActive(false);
                    confirmPasswordField.setActive(true);
                }
                else {
                    nameField.setActive(false);
                    surnameField.setActive(false);
                    pinField.setActive(false);
                    passwordField.setActive(false);
                    confirmPasswordField.setActive(false);
                }

                if (backButton.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {
                    signUpWindow.close();
                    showLoginWindow();
                }
                if (saveButton.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {
                    if (nameField.input.empty() || surnameField.input.empty() || pinField.input.empty() || passwordField.input.empty() || confirmPasswordField.input.empty()) {
                        errorText.setString("All fields are required.");
                        successText.setString("");
                    }
                    else if (passwordField.input != confirmPasswordField.input) {
                        errorText.setString("Passwords do not match.");
                        successText.setString("");
                    }
                    else if (passwordField.input.length() < 5) {
                        errorText.setString("Password must be at least 5 characters.");
                        successText.setString("");
                    }
                    else {
                        errorText.setString("");
                        json newUser = {
                            {"id", data["users"].size() + 1},
                            {"password", xorEncryptDecryptPassword(passwordField.input, "bankowa")},
                            {"id_user", xorEncryptDecryptPassword(pinField.input, "pinokio")},
                            {"name", nameField.input},
                            {"surname", surnameField.input}
                        };

                        data["users"].push_back(newUser);

                        ofstream outputFile(filePath);
                        outputFile << data.dump(4);
                        outputFile.close();

                        nameField.input.clear();
                        surnameField.input.clear();
                        pinField.input.clear();
                        passwordField.input.clear();
                        confirmPasswordField.input.clear();

                        nameField.text.setString("");
                        surnameField.text.setString("");
                        pinField.text.setString("");
                        passwordField.text.setString("");
                        confirmPasswordField.text.setString("");

                        generatedPin = generateUniquePin(data);
                        pinField.input = generatedPin;
                        pinField.text.setString(generatedPin);

                        successText.setString("Account created successfully!");
                    }
                }
            }

            nameField.handleEvent(event);
            surnameField.handleEvent(event);
            pinField.handleEvent(event);
            passwordField.handleEvent(event);
            confirmPasswordField.handleEvent(event);
        }

        signUpWindow.clear(sf::Color(0x38, 0xB6, 0xFF));

        signUpWindow.draw(logoSprite);
        nameField.draw(signUpWindow);
        surnameField.draw(signUpWindow);
        pinField.draw(signUpWindow);
        passwordField.draw(signUpWindow);
        confirmPasswordField.draw(signUpWindow);
        signUpWindow.draw(backButton);
        signUpWindow.draw(saveButton);
        signUpWindow.draw(backText);
        signUpWindow.draw(saveText);
        signUpWindow.draw(nameLabel);
        signUpWindow.draw(surnameLabel);
        signUpWindow.draw(pinLabel);
        signUpWindow.draw(passwordLabel);
        signUpWindow.draw(confirmPasswordLabel);
        signUpWindow.draw(errorText);
        signUpWindow.draw(successText);
        signUpWindow.draw(rightImageSprite);

        signUpWindow.display();
    }
}
