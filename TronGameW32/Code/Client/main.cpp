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
void PlayerDead();
void waitForValidID(ClientNetwork& CN, sf::Clock clock); 
//bool MainMenu(MainMenuSelection &MMS, sf::Sprite &JoinGame);
sf::Packet Game(ClientNetwork& CN, std::vector<sf::CircleShape>& grid, const int gridWidth, const int gridHeight, sf::Clock clock, std::vector<User> &users);
int lobby(ClientNetwork& CN);
void init(ClientNetwork& CN, SceneSelector &SS, sf::Sprite &JoinGame, std::vector<sf::CircleShape> &players, int window_x, int window_y, std::vector<sf::CircleShape> &grid, std::vector<User> &users);
sf::Color blankSpace = sf::Color::Black;
sf::Color trailColour = sf::Color::Red;
sf::Color deathColour = sf::Color::Cyan;
sf::Color winnerColour;
int main()
{
	sf::Texture t_Lobby;
	sf::Sprite JoinGame;
	if (!t_Lobby.loadFromFile("..\\..\\Resources\\Lobby.png"))
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
	JoinGame.setTexture(t_Lobby);
	init(*CN, SS, JoinGame, players, window.getSize().x, window.getSize().y, grid, users);
	CN->UpdateNumClients();

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
				init(*CN, SS, JoinGame, players, 0, 0, grid, users);
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
			window.draw(JoinGame);
			break;
		case SceneSelector::GAME:
			packet = Game(*CN, grid, 30, 30, clock, users); 
			if (packet.getDataSize() == 12)
			{ 
				while (!sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) || !sf::Joystick::isButtonPressed(CN->getClientNum(), 1)) {}
				CN->sendPacket(packet);
				init(*CN, SS, JoinGame, players, window.getSize().x, window.getSize().y, grid, users);
				SS = SceneSelector::LOBBY;
				clock.restart();
				for (int i = 0; i < grid.size(); i++)
				{
					grid[i].setFillColor(winnerColour);
					window.draw(grid[i]);
					while (clock.getElapsedTime().asSeconds() < 0.1){}
					clock.restart(); 
				}
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
void init(ClientNetwork& CN, SceneSelector &SS, sf::Sprite &JoinGame, std::vector<sf::CircleShape> &players, int window_x, int window_y, std::vector<sf::CircleShape> &grid, std::vector<User> &users)
{
	int gridWidth = 30;
	int gridHeight = 30;
	float shapeSize = 10.0f;
	switch (SS)
	{
	case SceneSelector::LOBBY:
		players.clear(); 
		JoinGame.setColor(sf::Color::White);
		JoinGame.setPosition(150, 0);
		for (int i = 0; i < 4; i++)
		{
			sf::CircleShape Player(50);
			Player.setFillColor(sf::Color::Red);
			Player.setPosition(window_x / 2, (window_y / 5)* (i + 1));
			players.push_back(Player);
		}
		break;
	case SceneSelector::GAME:
		grid.clear(); 
		grid.reserve(gridWidth*gridHeight);
		for (int i = 0; i < gridWidth*gridHeight; i++)
		{
			int x = i % gridWidth;
			int y = i / gridWidth;
			sf::CircleShape shape(shapeSize);
			shape.setFillColor(blankSpace);
			shape.setPosition(((shapeSize * 2)*x), ((shapeSize * 2)*y));
			grid.push_back(shape);
		}
		users.clear(); 
		users.reserve(4);
		for (int i = 0; i < CN.requestNumClients(); i++)
		{
			User* user = new User();
			user->setPos(i * (gridWidth*gridHeight / 4));
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
sf::Packet Game(ClientNetwork& CN, std::vector<sf::CircleShape> &grid, const int gridWidth, const int gridHeight, sf::Clock clock, std::vector<User> &users)
{
	int usersDead = 0; 
	std::string dead = "X";
	sf::Packet packet; 
	for (int i = 0; i < users.size(); i++)
	{
		if (!CN.heartBeatPlayer(i))
			users[i].setAlive(false); 
		if (users[i].getAlive() == false)
			usersDead++; 
	}
	if (usersDead == users.size() - 1)
	{
		for (int i = 0; i < users.size(); i++)
			if (users[i].getAlive() == true)
				winnerColour = users[i].getColour();
	}
	if (usersDead == users.size())
	{
		dead = "GAMEOVER";
		packet << dead;
		return packet; 
	}
		
	while (!(clock.getElapsedTime().asSeconds() > 0.1)) {}
	for (int i = 0; i < users.size(); i++)
	{
		if (users[i].getAlive() == true)
		{
			int pos = users[i].getPos();
			packet << dead;
			dead = std::to_string(i);
			packet << dead;
			switch (CN.getCMD(i))
			{
			case 'L':
				if (pos % gridWidth == 0)
				{
					grid[pos].setFillColor(deathColour);
					users[i].setAlive(false);
					return packet;
				}
				pos--;
				users[i].setPos(pos);
				if (grid[pos].getFillColor() != blankSpace)
				{
					grid[pos].setFillColor(deathColour);
					users[i].setAlive(false);
					return packet;
				}
				else
					grid[pos].setFillColor(users[i].getColour());
				break;
			case 'R':
				pos++;
				users[i].setPos(pos);
				if (pos % gridWidth == 0)
				{
					pos--;
					grid[pos].setFillColor(deathColour);
					users[i].setAlive(false);
					return packet;
				}
				if (grid[pos].getFillColor() != blankSpace)
				{
					grid[pos].setFillColor(deathColour);
					users[i].setAlive(false);
					return packet;
				}
				else
					grid[pos].setFillColor(users[i].getColour());
				break;
			case 'U':
				pos -= gridWidth;
				users[i].setPos(pos);
				if (pos < 0)
				{
					grid[pos += gridWidth].setFillColor(deathColour);
					users[i].setAlive(false);
					return packet;
				}
				if (grid[pos].getFillColor() != blankSpace)
				{
					grid[pos].setFillColor(deathColour);
					users[i].setAlive(false);
					return packet;
				}
				else
					grid[pos].setFillColor(users[i].getColour());
				break;
			case 'D':
				pos += gridWidth;
				users[i].setPos(pos);
				if (pos >= (gridWidth*gridHeight))
				{
					grid[pos -= gridWidth].setFillColor(deathColour);
					users[i].setAlive(false);
					return packet;
				}
				if (grid[pos].getFillColor() != blankSpace)
				{
					grid[pos].setFillColor(deathColour);
					users[i].setAlive(false);
					return packet;
				}
				else
					grid[pos].setFillColor(users[i].getColour());
				break;
			}
		}
	}
		packet.clear(); 
		return packet; 
}

void PlayerDead()
{
	int msgboxID = MessageBox(NULL, (LPCWSTR)L"TROUBLE OBTAINING HOST\nATTEMPTING TO CONNECT", (LPCWSTR)L"ERROR", MB_ICONWARNING);
}

void waitForValidID(ClientNetwork& CN, sf::Clock clock)
{
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
			PlayerDead();
			patience += 10; 
		}
	}
}

//while (stayOpen)
//{

//}

//while (window.isOpen())
//{
//	while (MainMenu(MMS, JoinGame) == false)
//	{
//		window.clear();
//		window.draw(JoinGame);
//		window.display();
//	}
//	if (MMS == MainMenuSelection::QUIT)
//		window.close();
//	ClientNetwork* CN = new ClientNetwork();
//	std::thread networking(&ClientNetwork::client, CN);
//	waitForValidID(*CN);
//	JoinGame.setTexture(t_Lobby);
//	JoinGame.setColor(sf::Color::White);
//	JoinGame.setPosition(150, 0);
//	std::vector<sf::CircleShape> players;
//	sf::Clock clock;
//	CN->UpdateNumClients();
//	while (lobby(*CN) == false)
//	{
//		window.clear();
//		if (clock.getElapsedTime().asSeconds() > 2)
//		{
//			for (int i = 0; i < CN->requestNumClients(); i++)
//			{
//				players[i].setFillColor(sf::Color::Green);
//			}
//			clock.restart();
//		}
//		for (int i = 0; i < 4; i++)
//		{
//			window.draw(players[i]);
//		}
//		window.draw(JoinGame);
//		window.display();
//	}
//	std::vector<sf::CircleShape> grid;
//	int gridWidth = 30;
//	int gridHeight = 30;
//	grid.reserve(gridWidth*gridHeight);
//	float shapeSize = 10.0f;
//	for (int i = 0; i < gridWidth*gridHeight; i++)
//	{
//		int x = i % gridWidth;
//		int y = i / gridWidth;
//
//		sf::CircleShape shape(shapeSize);
//		shape.setFillColor(sf::Color::Green);
//		shape.setPosition(((shapeSize * 2)*x), ((shapeSize * 2)*y));
//		grid.push_back(shape);
//	}
//	std::vector<User> users;
//	users.reserve(4);
//	for (int i = 0; i < CN->requestNumClients(); i++)
//	{
//		User* user = new User();
//		user->setPos(CN->getClientNum() * (gridWidth*gridHeight / 4));
//		users.push_back(*user);
//		grid[users[i].getPos()].setFillColor(sf::Color::Blue);
//	}
//	while (Game(*CN, grid, gridWidth, gridHeight, clock, users) == false)
//	{
//		window.clear();
//		for (int i = 0; i < grid.size(); i++)
//		{
//			window.draw(grid[i]);
//		}
//		window.display();
//		clock.restart();
//	}
//	window.display();
//}

		//case SceneSelector::MENU:
		//	if (MainMenu(MMS, JoinGame) == true)
		//	{
		//		if (MMS == MainMenuSelection::QUIT)
		//		{
		//			window.close();
		//			break;
		//		}
		//		SS = SceneSelector::LOBBY;
		//		new std::thread(&ClientNetwork::client, CN);
		//		waitForValidID(*CN);
		//		JoinGame.setTexture(t_Lobby);
		//		init(*CN, SS, JoinGame, players, window.getSize().x, window.getSize().y, grid, users);
		//		CN->UpdateNumClients();
		//	}
		//	window.draw(JoinGame);
		//	break;

//bool MainMenu(MainMenuSelection &MMS, sf::Sprite &JoinGame)
//{
//	switch (MMS)
//	{
//	case MainMenuSelection::JOIN_GAME:
//		JoinGame.setColor(sf::Color::Green);
//		break;
//	case MainMenuSelection::QUIT:
//		JoinGame.setColor(sf::Color::White);
//		break;
//	default:
//		break;
//	}
//
//	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
//	{
//		if (MMS == MainMenuSelection::JOIN_GAME)
//			MMS = MainMenuSelection::QUIT;
//		else
//			MMS = MainMenuSelection::JOIN_GAME;
//	}
//	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
//		return true;
//	return false;
//}