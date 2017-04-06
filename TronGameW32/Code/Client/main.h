#pragma once
#include <iostream>
#include <future>
#include <string>
#include <thread>
#include <SFML\Network.hpp>
#include <atomic>
#include "Game\User.h"
#include "ClientNetwork.h"
#include "Game\User.h"
enum class SceneSelector
{
	LOBBY,
	GAME
};

class main
{
public:
	main() = default; 
	virtual ~main() = default;
	void PlayerDead(std::vector<User> &users, const int i);
	void waitForValidID(ClientNetwork& CN);
	int Game(); 
	//bool MainMenu(MainMenuSelection &MMS, sf::Sprite &JoinGame);
	sf::Packet Game(ClientNetwork& CN, std::vector<sf::CircleShape>& grid, const int gridWidth, const int gridHeight, sf::Clock clock, std::vector<User> &users);
	int lobby(ClientNetwork& CN);
	void init(ClientNetwork& CN, SceneSelector &SS, sf::Sprite &JoinGame, std::vector<sf::CircleShape> &players, int window_x, int window_y, std::vector<sf::CircleShape> &grid, std::vector<User> &users);

private:
	sf::Color blankSpace = sf::Color::Green;
	sf::Color trailColour = sf::Color::Red;
	sf::Color deathColour = sf::Color::Cyan;
};