#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <random>
#include "Header.h"

using namespace std;
using json = nlohmann::json;


void showUserMainWindows(const string& userName) {
    sf::RenderWindow window(sf::VideoMode(1000, 500), "User Main Window", sf::Style::Titlebar | sf::Style::Close);
    sf::Font font;

    if (!font.loadFromFile("arial.ttf")) {
        cout << "Could not load font\n";
        return;
    }

    sf::RectangleShape createAccountButton(sf::Vector2f(200.f, 50.f));
    createAccountButton.setPosition(400, 430);
    createAccountButton.setFillColor(sf::Color::Green);

    sf::Text createAccountText("Create Account", font, 20);
    createAccountText.setPosition(415, 445);
    createAccountText.setFillColor(sf::Color::White);

    sf::RectangleShape signOutButton(sf::Vector2f(100.f, 50.f));
    signOutButton.setPosition(880, 10);
    signOutButton.setFillColor(sf::Color::Red);

    sf::Text signOutText("Sign Out", font, 20);
    signOutText.setPosition(885, 25);
    signOutText.setFillColor(sf::Color::White);

    sf::Text welcomeText("Welcome, " + userName, font, 20);
    welcomeText.setPosition(10, 10);
    welcomeText.setFillColor(sf::Color::White);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
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
                }
            }
        }

        window.clear(sf::Color::Black);
        window.draw(createAccountButton);
        window.draw(createAccountText);
        window.draw(signOutButton);
        window.draw(signOutText);
        window.draw(welcomeText);
        window.display();
    }
}
