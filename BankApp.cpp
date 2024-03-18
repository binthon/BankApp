#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;
int main() {
    sf::RenderWindow window(sf::VideoMode(400, 200), "Formularz SFML");

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
    submitButton.setPosition(10.f, 100.f);
    submitButton.setFillColor(sf::Color::Red);
    submitButton.setOutlineColor(sf::Color::Black);
    submitButton.setOutlineThickness(2.f);

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

    sf::Text submitText("Submit", font, 20);
    submitText.setPosition(15.f, 100.f);
    submitText.setFillColor(sf::Color::White);

    std::string inputText1 = "";
    std::string inputText2 = "";
    bool isTextField1Active = false;
    bool isTextField2Active = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (submitButton.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                    json j;
                    j["login"] = inputText1;
                    j["password"] = inputText2;

                    ofstream outputFile("loginData.json");
                    outputFile << j.dump(4);
                    outputFile.close();

                    cout << "Data saved to loginData.json" << endl;
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
                }
                else if (isTextField2Active) {
                    if (event.text.unicode == '\b' && !inputText2.empty()) { 
                        inputText2.pop_back();
                    }
                    else if (event.text.unicode < 128 && event.text.unicode != '\b') { 
                        inputText2 += static_cast<char>(event.text.unicode);
                    }
                }
                text1.setString(inputText1);
                text2.setString(inputText2);
            }
        }

        window.clear();
        window.draw(textField1);
        window.draw(textField2);
        window.draw(submitButton);
        window.draw(text1);
        window.draw(text2);
        window.draw(submitText);
        window.display();
    }

    return 0;
}
