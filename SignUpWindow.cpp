#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include "Header.h"
#include <random>

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
    sf::RectangleShape box;
    sf::Text text;
    string input;
    bool isActive = false;
    bool isEditable = true;

    TextField(float x, float y, const sf::Font& font, unsigned int fontSize = 20, bool editable = true)
        : isEditable(editable) {
        box.setSize(sf::Vector2f(200.f, 30.f));
        box.setPosition(x, y);
        box.setFillColor(sf::Color::Magenta);
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

    void draw(sf::RenderWindow& window) {
        window.draw(box);
        window.draw(text);
    }
};

struct PasswordField {
    sf::RectangleShape box;
    sf::Text text;
    string input;
    bool isActive = false;

    PasswordField(float x, float y, const sf::Font& font, unsigned int fontSize = 20) {
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
    sf::RenderWindow signUpWindow(sf::VideoMode(1000, 500), "Sign Up", sf::Style::Titlebar | sf::Style::Close);
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "Could not load font\n";
        return;
    }

    string generatedPin = generateRandomPin();
    TextField nameField(400, 50, font);
    TextField surnameField(400, 100, font);
    TextField pinField(400, 150, font, 20, false);
    PasswordField passwordField(400, 200, font, 20);
    PasswordField confirmPasswordField(400, 250, font, 20);

    sf::RectangleShape backButton(sf::Vector2f(100.f, 50.f));
    backButton.setPosition(50, 400);
    backButton.setFillColor(sf::Color::Green);

    sf::RectangleShape saveButton(sf::Vector2f(100.f, 50.f));
    saveButton.setPosition(850, 400);
    saveButton.setFillColor(sf::Color::Red);

    sf::Text backText("Back", font, 20);
    backText.setPosition(60, 410);
    backText.setFillColor(sf::Color::White);

    sf::Text saveText("Save", font, 20);
    saveText.setPosition(860, 410);
    saveText.setFillColor(sf::Color::White);

    sf::Text nameLabel("Name:", font, 20);
    nameLabel.setPosition(300, 50);
    nameLabel.setFillColor(sf::Color::White);

    sf::Text surnameLabel("Surname:", font, 20);
    surnameLabel.setPosition(300, 100);
    surnameLabel.setFillColor(sf::Color::White);

    sf::Text pinLabel("ID:", font, 20);
    pinLabel.setPosition(300, 150);
    pinLabel.setFillColor(sf::Color::White);

    sf::Text passwordLabel("Password:", font, 20);
    passwordLabel.setPosition(300, 200);
    passwordLabel.setFillColor(sf::Color::White);

    sf::Text confirmPasswordLabel("Confirm Password:", font, 20);
    confirmPasswordLabel.setPosition(220, 250);
    confirmPasswordLabel.setFillColor(sf::Color::White);

    sf::Text errorText("", font, 20);
    errorText.setPosition(400, 300);
    errorText.setFillColor(sf::Color::Red);

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
                    }
                    else if (passwordField.input != confirmPasswordField.input) {
                        errorText.setString("Passwords do not match.");
                    }
                    else if (passwordField.input.length() < 5) {
                        errorText.setString("Password must be at least 5 characters.");
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
                    }
                }
            }

            nameField.handleEvent(event);
            surnameField.handleEvent(event);
            pinField.handleEvent(event);
            passwordField.handleEvent(event);
            confirmPasswordField.handleEvent(event);
        }

        signUpWindow.clear(sf::Color::Black);
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

        signUpWindow.display();
    }
}
