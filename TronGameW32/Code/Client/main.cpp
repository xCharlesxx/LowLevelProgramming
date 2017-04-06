#include <SFML/Graphics.hpp>
#include <iostream>
#include <future>
#include <string>
#include <SFML\Network.hpp>
#include "ClientNetwork.h"
#include "Game\User.h"
#include <Windows.h>

enum class SceneSelector
{
	LOBBY,
	GAME
};
void playerDead();
void waitForValidID(ClientNetwork& CN, sf::Clock clock); 
//bool MainMenu(MainMenuSelection &MMS, sf::Sprite &join_game);
sf::Packet game(ClientNetwork& CN, std::vector<sf::CircleShape>& grid, const int grid_width, const int grid_height, sf::Clock clock, std::vector<User> &users);
int lobby(ClientNetwork& CN);
void init(ClientNetwork& CN, SceneSelector &SS, sf::Sprite &join_game, std::vector<sf::CircleShape> &players, int window_x, int window_y, std::vector<sf::CircleShape> &grid, std::vector<User> &users);
sf::Color blank_space = sf::Color::Black;
sf::Color trail_colour = sf::Color::Red;
sf::Color death_colour = sf::Color::Cyan;
sf::Color winner_colour;
const int grid_width = 30;
const int grid_height = 30;
const float shape_size = 10.0f;

int main()
{
	sf::Texture t_lobby;
	sf::Sprite join_game;
	if (!t_lobby.loadFromFile("..\\..\\Resources\\Lobby.png"))
	{
		std::cout << "NO";
	}
	sf::RenderWindow window(sf::VideoMode(600, 600), "TRON");

	SceneSelector SS = SceneSelector::LOBBY; 
	ClientNetwork* CN = new ClientNetwork();
	std::vector<User> users;
	std::vector<sf::CircleShape> grid;
	std::vector<sf::CircleShape> players;
	sf::Packet packet;
	sf::Clock clock;
	new std::thread(&ClientNetwork::client, CN);
	waitForValidID(*CN, clock);
	join_game.setTexture(t_lobby);
	init(*CN, SS, join_game, players, window.getSize().x, window.getSize().y, grid, users);
	CN->updateNumClients();

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		window.clear();
		
		switch (SS)
		{
		case SceneSelector::LOBBY:
			//Check lobby contents every two seconds
			if (clock.getElapsedTime().asSeconds() > 1)
			{
				for (int i = 0; i < CN->requestNumClients(); i++)
				{
					players[i].setFillColor(sf::Color::Green);
				}
				clock.restart();
			}
			//Check if game has been started
			if (lobby(*CN) == 1)
			{
				SS = SceneSelector::GAME;
				CN->requestNumClients();
				init(*CN, SS, join_game, players, 0, 0, grid, users);
			}
			else if (lobby(*CN) == 2)
			{
				CN->disconnect(); 
				window.close(); 
			}
			for (int i = 0; i < 4; i++)
			{
				window.draw(players[i]);
			}
			window.draw(join_game);
			break;
		case SceneSelector::GAME:
			packet = game(*CN, grid, 30, 30, clock, users); 
			if (packet.getDataSize() == 12)
			{ 
				clock.restart();
				for (int i = 0; i < grid.size(); i++)
				{
					grid[i].setFillColor(winner_colour);
					window.draw(grid[i]);
					window.display(); 
					while (clock.getElapsedTime().asSeconds() < 0.005) {}
					clock.restart();
				}
				CN->sendPacket(packet);
				
				init(*CN, SS, join_game, players, window.getSize().x, window.getSize().y, grid, users);
				SS = SceneSelector::LOBBY;
				break; 
			}
				 
			if (packet.getDataSize() != 0)
			CN->sendPacket(packet); 
			if (CN->getClientNum() == 0)
			CN->sendPacket(packet << "Ping");
			CN->clearCMD();
			for (int i = 0; i < grid.size(); i++)
			{
				window.draw(grid[i]);
			}
			clock.restart();
			break;
		default:
			break;
		}
		window.display();
	}
	return 0; 
}
void init(ClientNetwork& CN, SceneSelector &SS, sf::Sprite &join_game, std::vector<sf::CircleShape> &players, int window_x, int window_y, std::vector<sf::CircleShape> &grid, std::vector<User> &users)
{
	switch (SS)
	{
	case SceneSelector::LOBBY:
		players.clear(); 
		join_game.setColor(sf::Color::White);
		join_game.setPosition(150, 0);
		for (int i = 0; i < 4; i++)
		{
			sf::CircleShape player(50);
			player.setFillColor(sf::Color::Red);
			player.setPosition(window_x / 2, (window_y / 5)* (i + 1));
			players.push_back(player);
		}
		break;
	case SceneSelector::GAME:
		grid.clear(); 
		grid.reserve(grid_width*grid_height);
		for (int i = 0; i < grid_width*grid_height; i++)
		{
			int x = i % grid_width;
			int y = i / grid_width;
			sf::CircleShape shape(shape_size);
			shape.setFillColor(blank_space);
			shape.setPosition(((shape_size * 2)*x), ((shape_size * 2)*y));
			grid.push_back(shape);
		}
		users.clear(); 
		users.reserve(4);
		for (int i = 0; i < CN.requestNumClients(); i++)
		{
			User* user = new User();
			user->setPos(i * (grid_width*grid_height / 4));
			user->setAlive(true); 
			switch (i)
			{
			case 0:
				user->setColour(sf::Color::Red);
				break;
			case 1:
				user->setColour(sf::Color::Green);
				break;
			case 2:
				user->setColour(sf::Color::Magenta);
				break;
			case 3:
				user->setColour(sf::Color::Yellow);
				break;
			default:
				break;
			}
			users.push_back(*user);
			grid[users[i].getPos()].setFillColor(users[i].getColour());
		}
		break;
	}
}

int lobby(ClientNetwork& CN)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) || sf::Joystick::isButtonPressed(CN.getClientNum(), 1))
		return 2;
	if (CN.checkGameStart() == true)
		return 1; 
	return 0;
}
sf::Packet game(ClientNetwork& CN, std::vector<sf::CircleShape> &grid, const int grid_width, const int grid_height, sf::Clock clock, std::vector<User> &users)
{
	//If player gets stuck in game loop, Escape can be used to end the game
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) || sf::Joystick::isButtonPressed(CN.getClientNum(), 1)) 
	{
		for (int i = 0; i < users.size(); i++)
			users[i].setAlive(false);
	}
	int users_dead = 0; 
	std::string dead = "X";
	sf::Packet packet; 
	//Check which players are alive
	for (int i = 0; i < users.size(); i++)
	{
		if (!CN.heartBeatplayer(i))
			users[i].setAlive(false); 
		if (users[i].getAlive() == false)
			users_dead++; 
	}
	//If all but one players are dead, that is the winner
	if (users_dead == users.size() - 1)
	{
		for (int i = 0; i < users.size(); i++)
			if (users[i].getAlive() == true)
				winner_colour = users[i].getColour();
	}
	//If everyone is dead then game over
	if (users_dead == users.size())
	{
		dead = "GAMEOVER";
		packet << dead;
		return packet; 
	}
	//Wait for a delay before moving the player
	while (!(clock.getElapsedTime().asSeconds() > 0.1)) {}
	for (int i = 0; i < users.size(); i++)
	{
		//If player is alive
		if (users[i].getAlive() == true)
		{
			int pos = users[i].getPos();
			packet << dead;
			dead = std::to_string(i);
			packet << dead;
			//Get most recent command
			switch (CN.getCMD(i))
			{
			case 'L':
				//Check if player has hit a wall
				if (pos % grid_width == 0)
				{
					grid[pos].setFillColor(death_colour);
					users[i].setAlive(false);
					return packet;
				}
				//Otherwise move
				pos--;
				users[i].setPos(pos);
				//Check if player has collided with other player trail
				if (grid[pos].getFillColor() != blank_space)
				{
					grid[pos].setFillColor(death_colour);
					users[i].setAlive(false);
					return packet;
				}
				else
					grid[pos].setFillColor(users[i].getColour());
				break;
			case 'R':
				pos++;
				users[i].setPos(pos);
				if (pos % grid_width == 0)
				{
					pos--;
					grid[pos].setFillColor(death_colour);
					users[i].setAlive(false);
					return packet;
				}
				if (grid[pos].getFillColor() != blank_space)
				{
					grid[pos].setFillColor(death_colour);
					users[i].setAlive(false);
					return packet;
				}
				else
					grid[pos].setFillColor(users[i].getColour());
				break;
			case 'U':
				pos -= grid_width;
				users[i].setPos(pos);
				if (pos < 0)
				{
					grid[pos += grid_width].setFillColor(death_colour);
					users[i].setAlive(false);
					return packet;
				}
				if (grid[pos].getFillColor() != blank_space)
				{
					grid[pos].setFillColor(death_colour);
					users[i].setAlive(false);
					return packet;
				}
				else
					grid[pos].setFillColor(users[i].getColour());
				break;
			case 'D':
				pos += grid_width;
				users[i].setPos(pos);
				if (pos >= (grid_width*grid_height))
				{
					grid[pos -= grid_width].setFillColor(death_colour);
					users[i].setAlive(false);
					return packet;
				}
				if (grid[pos].getFillColor() != blank_space)
				{
					grid[pos].setFillColor(death_colour);
					users[i].setAlive(false);
					return packet;
				}
				//Else all clear and move player
				else
					grid[pos].setFillColor(users[i].getColour());
				break;
			}
		}
	}
		packet.clear(); 
		return packet; 
}

void playerDead()
{
	int msgboxID = MessageBox(NULL, (LPCWSTR)L"TROUBLE OBTAINING HOST\nATTEMPTING TO CONNECT", (LPCWSTR)L"ERROR", MB_ICONWARNING);
}

void waitForValidID(ClientNetwork& CN, sf::Clock clock)
{
	//Wait until a valid client ID is secured from the Server
	int patience = 5; 
	bool b = true;
	while (b)
	{
		if (CN.getClientNum() < 5)
		{
			b = false;
		}
		if (clock.getElapsedTime().asSeconds() > patience)
		{
			playerDead();
			patience += 10; 
		}
	}
}

