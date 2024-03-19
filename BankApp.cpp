#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
 

using namespace std;
using json = nlohmann::json;

string xorEncryptDecrypt(const string& input, const string& key) {
    string output = input;
    size_t keyLength = key.length();
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = input[i] ^ key[i % keyLength];
    }

    return output;
}


int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 500), "FORM");

    sf::RectangleShape textField1(sf::Vector2f(180.f, 30.f));
    textField1.setPosition(10.f, 10.f);
    textField1.setFillColor(sf::Color::White);
    textField1.setOutlineColor(sf::Color::Black);
    textField1.setOutlineThickness(2.f);

    sf::RectangleShape textField2(sf::Vector2f(180.f, 30.f));
    textField2.setPosition(10.f, 50.f);
    textField2.setFillColor(sf::Color::White);
    textField2.setOutlineColor(sf::Color::Black);
    textField2.setOutlineThickness(2.f);

    sf::RectangleShape submitButton(sf::Vector2f(100.f, 30.f));
    submitButton.setPosition(105.f, 100.f);
    submitButton.setFillColor(sf::Color::Red);
    submitButton.setOutlineColor(sf::Color::Black);
    submitButton.setOutlineThickness(2.f);

    sf::RectangleShape registerButton(sf::Vector2f(100.f, 30.f));
    registerButton.setPosition(10.f, 100.f);
    registerButton.setFillColor(sf::Color::Green);
    registerButton.setOutlineColor(sf::Color::Black);
    registerButton.setOutlineThickness(2.f);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        return -1;
    }

    sf::Text text1("", font, 20);
    text1.setPosition(15.f, 10.f);
    text1.setFillColor(sf::Color::Black);

    sf::Text text2("", font, 20);
    text2.setPosition(15.f, 50.f);
    text2.setFillColor(sf::Color::Black);

    string passwordDisplay = ""; 
    string actualPassword = "";

    sf::Text submitText("SIGN IN", font, 20);
    submitText.setPosition(115.f, 100.f);
    submitText.setFillColor(sf::Color::White);

    sf::Text registerText("SIGN UP", font, 20);
    registerText.setPosition(15.f, 100.f); 
    registerText.setFillColor(sf::Color::White);

    std::string inputText1 = "";
    std::string inputText2 = "";
    bool isTextField1Active = false;
    bool isTextField2Active = false;

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
                if (submitButton.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                    bool loginSuccess = false; 

                    if (inputText1.empty() || actualPassword.empty()) {
                        cout << "Empty login or password" << endl;
                    }
                    else {
                        
                        string encryptedPassword = xorEncryptDecrypt(actualPassword, "X");

                       
                        for (const auto& user : existingData["users"]) {
                            if (user["login"] == inputText1 && user["password"] == encryptedPassword) {
                                loginSuccess = true;
                                break;
                            }
                        }
                        if (loginSuccess) {
                            cout << "Login successful" << endl;
                        }
                        else {
                            cout << "Login failed: invalid login or password" << endl;
                        }
                        inputText1 = "";
                        inputText2 = "";
                        actualPassword = "";
                        passwordDisplay = "";
                        text1.setString(inputText1);
                        text2.setString(passwordDisplay);
                    }
                }
                else if (registerButton.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                   
                    cout << "Click sing up" << endl;
                }
                else if (textField1.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                    isTextField1Active = true;
                    isTextField2Active = false;
                    textField1.setOutlineColor(sf::Color::Blue);
                    textField2.setOutlineColor(sf::Color::Black);
                }
                else if (textField2.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                    isTextField1Active = false;
                    isTextField2Active = true;
                    textField1.setOutlineColor(sf::Color::Black);
                    textField2.setOutlineColor(sf::Color::Blue);
                }
                else {
                    isTextField1Active = false;
                    isTextField2Active = false;
                    textField1.setOutlineColor(sf::Color::Black);
                    textField2.setOutlineColor(sf::Color::Black);
                }
            }

            if (event.type == sf::Event::TextEntered) {
                if (isTextField1Active) {
                    if (event.text.unicode == '\b' && !inputText1.empty()) {
                        inputText1.pop_back();
                    }
                    else if (event.text.unicode < 128 && event.text.unicode != '\b') {
                        inputText1 += static_cast<char>(event.text.unicode);
                    }
                    text1.setString(inputText1);
                }
                else if (isTextField2Active) {
                    if (event.text.unicode == '\b' && !actualPassword.empty()) {
                        actualPassword.pop_back();
                        passwordDisplay.pop_back();
                    }
                    else if (event.text.unicode < 128 && event.text.unicode != '\b') {
                        actualPassword += static_cast<char>(event.text.unicode);
                        passwordDisplay += '*';
                    }
                    text2.setString(passwordDisplay); 
                }
       
            }
        }

        window.clear();
        window.draw(textField1);
        window.draw(textField2);
        window.draw(submitButton);
        window.draw(registerButton);
        window.draw(registerText);
        window.draw(text1);
        window.draw(text2);
        window.draw(submitText);
        window.display();
    }

    return 0;
}
//fsdfdfd odszyfrowane hasło do user ID=3