#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "Header.h" 
#include <random>
using json = nlohmann::json;
using namespace std;


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
    for (int i = 0; i < 4; ++i) {
        pin += to_string(distr(generator));
    }

    return pin;
}

struct TextField {
    sf::RectangleShape box;
    sf::Text text;
    string input;
    bool isActive = false;
    bool isEditable = true;

    TextField(float x, float y, const sf::Font& font, unsigned int fontSize = 20, bool editable = true) : isEditable(editable) {
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
    TextField loginField(400, 200, font);
    TextField passwordField(400, 250, font);
    pinField.input = generatedPin;
    pinField.text.setString(pinField.input);
   
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

    sf::Text pinLabel("PIN:", font, 20);
    pinLabel.setPosition(300, 150);
    pinLabel.setFillColor(sf::Color::White);

    sf::Text loginLabel("Login:", font, 20);
    loginLabel.setPosition(300, 200);
    loginLabel.setFillColor(sf::Color::White);

    sf::Text passwordLabel("Password:", font, 20);
    passwordLabel.setPosition(300, 250);
    passwordLabel.setFillColor(sf::Color::White);


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
                    loginField.setActive(false);
                    passwordField.setActive(false);
                }
                else if (surnameField.box.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {
                    nameField.setActive(false);
                    surnameField.setActive(true);
                    pinField.setActive(false);
                    loginField.setActive(false);
                    passwordField.setActive(false);
                }
                else if (pinField.box.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {
                    nameField.setActive(false);
                    surnameField.setActive(false);
                    pinField.setActive(true);
                    loginField.setActive(false);
                    passwordField.setActive(false);
                }
                else if (loginField.box.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {
                    nameField.setActive(false);
                    surnameField.setActive(false);
                    pinField.setActive(false);
                    loginField.setActive(true);
                    passwordField.setActive(false);
                }
                else if (passwordField.box.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {
                    nameField.setActive(false);
                    surnameField.setActive(false);
                    pinField.setActive(false);
                    loginField.setActive(false);
                    passwordField.setActive(true);
                }
                else {

                    nameField.setActive(false);
                    surnameField.setActive(false);
                    pinField.setActive(false);
                    loginField.setActive(false);
                    passwordField.setActive(false);
                }

                if (backButton.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {

                    signUpWindow.close();
                    showLoginWindow();
                }
                if (saveButton.getGlobalBounds().contains(signUpWindow.mapPixelToCoords(sf::Mouse::getPosition(signUpWindow)))) {
                    if (nameField.input.empty() || surnameField.input.empty() || pinField.input.empty() || loginField.input.empty() || passwordField.input.empty()) {
                        cout << "Empty any area" << endl;
                    }
                    else {
                        string filePath = "loginData.json";
                        ifstream inputFile(filePath);
                        json data;
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
                        int maxId = 0;
                        for (const auto& user : data["users"]) {
                            if (user["id"].get<int>() > maxId) {
                                maxId = user["id"].get<int>();
                            }
                        }

                        json newUser = {
                         {"id", maxId + 1},
                         {"login", loginField.input},
                         {"password", xorEncryptDecryptPassword(passwordField.input, "bankowa")},
                         {"pin", xorEncryptDecryptPassword(pinField.input, "pinokio")},
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
                        loginField.input.clear();
                        passwordField.input.clear();

                        nameField.text.setString("");
                        surnameField.text.setString("");
                        pinField.text.setString("");
                        loginField.text.setString("");
                        passwordField.text.setString("");
                    }

                }
            }
            
            nameField.handleEvent(event);
            surnameField.handleEvent(event);
            pinField.handleEvent(event);
            loginField.handleEvent(event);
            passwordField.handleEvent(event);
        }

        signUpWindow.clear(sf::Color::Black);
        signUpWindow.draw(nameField.box);
        signUpWindow.draw(nameField.text);
        signUpWindow.draw(surnameField.box);
        signUpWindow.draw(surnameField.text);
        signUpWindow.draw(pinField.box);
        signUpWindow.draw(pinField.text);
        signUpWindow.draw(loginField.box);
        signUpWindow.draw(loginField.text);
        signUpWindow.draw(passwordField.box);
        signUpWindow.draw(passwordField.text);
        signUpWindow.draw(backButton);
        signUpWindow.draw(saveButton);
        signUpWindow.draw(backText);
        signUpWindow.draw(saveText);
        signUpWindow.draw(nameLabel);
        signUpWindow.draw(surnameLabel);
        signUpWindow.draw(pinLabel);
        signUpWindow.draw(loginLabel);
        signUpWindow.draw(passwordLabel);

        signUpWindow.display();
    }
}
