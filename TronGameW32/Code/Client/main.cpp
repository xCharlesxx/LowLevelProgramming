#include <SFML/Graphics.hpp>
#include <iostream>
#include <future>
#include <string>
#include <SFML\Network.hpp>
#include "ClientNetwork.h"
#include "User.h"
#include <Windows.h>

enum class MainMenuSelection
{
	INVALID = -1,
	NONE = 0,
	JOIN_GAME,
	QUIT
};
void PlayerDead(); 
void waitForValidID(ClientNetwork& CN); 
bool MainMenu(MainMenuSelection &MMS, sf::Sprite &JoinGame);
bool Game(ClientNetwork& CN, std::vector<sf::CircleShape>& grid, const int gridWidth, const int gridHeight, sf::Clock clock, std::vector<User> &users);
bool lobby(ClientNetwork& CN);
int main()
{
	sf::Texture t_Start; 
	sf::Texture t_Lobby; 
	if (!t_Start.loadFromFile("..\\..\\Resources\\Start.png"))
	{
		std::cout << "NO"; 
	}
	if (!t_Lobby.loadFromFile("..\\..\\Resources\\Lobby.png"))
	{
		std::cout << "NO";
	}
	MainMenuSelection MMS;
	MMS = MainMenuSelection::JOIN_GAME;
	sf::RenderWindow window(sf::VideoMode(600, 600), "TRON");
	sf::Sprite JoinGame;
	JoinGame.setTexture(t_Start); 
	JoinGame.setColor(sf::Color::Green); 
	JoinGame.setPosition(150, 100); 
	while (window.isOpen())
	{
		while (MainMenu(MMS, JoinGame) == false)
		{
			window.clear();
			window.draw(JoinGame);
			window.display();
		}
		if (MMS == MainMenuSelection::QUIT)
			window.close(); 
		ClientNetwork* CN = new ClientNetwork();
		std::thread networking(&ClientNetwork::client, CN);
		waitForValidID(*CN);
		JoinGame.setTexture(t_Lobby);
		JoinGame.setColor(sf::Color::White); 
		JoinGame.setPosition(150, 0);
		std::vector<sf::CircleShape> players;
		for (int i = 0; i < 4; i++)
		{
			sf::CircleShape Player(50);
			Player.setFillColor(sf::Color::Red);
			Player.setPosition(window.getSize().x / 2, (window.getSize().y / 5)* (i+1));
			players.push_back(Player);
		}
		sf::Clock clock;
		CN->UpdateNumClients(); 
		while (lobby(*CN) == false)
		{
			window.clear();
			if (clock.getElapsedTime().asSeconds() > 2)
			{
				for (int i = 0; i < CN->requestNumClients(); i++)
				{
					players[i].setFillColor(sf::Color::Green);
				}
				clock.restart(); 
			}
			for (int i = 0; i < 4; i++)
			{
				window.draw(players[i]);
			}
			window.draw(JoinGame);
			window.display();
		}
		std::vector<sf::CircleShape> grid;
		int gridWidth = 30;
		int gridHeight = 30;
		grid.reserve(gridWidth*gridHeight);
		float shapeSize = 10.0f;
		for (int i = 0; i < gridWidth*gridHeight; i++)
		{
			int x = i % gridWidth;
			int y = i / gridWidth;

			sf::CircleShape shape(shapeSize);
			shape.setFillColor(sf::Color::Green);
			shape.setPosition(((shapeSize * 2)*x), ((shapeSize * 2)*y));
			grid.push_back(shape);
		}
		std::vector<User> users; 
		users.reserve(4); 
		for (int i = 0; i < CN->requestNumClients(); i++)
		{
			User* user = new User();
			user->setPos(CN->getClientNum() * (gridWidth*gridHeight / 4));
			users.push_back(*user); 
			grid[users[i].getPos()].setFillColor(sf::Color::Blue);
		}
		while (Game(*CN, grid, gridWidth, gridHeight, clock, users) == false)
		{
			window.clear();
			for (int i = 0; i < grid.size(); i++)
			{
				window.draw(grid[i]);
			}
			window.display();
			clock.restart();
		}
		window.display();
	}
}
bool MainMenu(MainMenuSelection &MMS, sf::Sprite &JoinGame)
{
	switch (MMS)
	{
	case MainMenuSelection::JOIN_GAME:
		JoinGame.setColor(sf::Color::Green);
		break;
	case MainMenuSelection::QUIT:
		JoinGame.setColor(sf::Color::White);
		break;
	default:
		break;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		if (MMS == MainMenuSelection::JOIN_GAME)
			MMS = MainMenuSelection::QUIT;
		else
			MMS = MainMenuSelection::JOIN_GAME;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
		return true; 
	return false; 
}
bool lobby(ClientNetwork& CN)
{
	return CN.checkGameStart();
}
bool Game(ClientNetwork& CN, std::vector<sf::CircleShape> &grid, const int gridWidth, const int gridHeight, sf::Clock clock, std::vector<User> &users)
{
	while (!(clock.getElapsedTime().asSeconds() > 0.1)) {}
	for (int i = 0; i < users.size(); i++)
	{
		int pos = users[i].getPos(); 
		switch (CN.getCMD())
		{
		case 'L':
			if (pos < 0 || pos % gridWidth == 0)
			{
				PlayerDead();
				//x += gridWidth;
				return true;
			}
			pos--;
			users[i].setPos(pos);
			if (grid[pos].getFillColor() == sf::Color::Red)
			{
				PlayerDead();
				grid[pos].setFillColor(sf::Color::Cyan);
				return true;
			}
			else
				grid[pos].setFillColor(sf::Color::Red);
			break;
		case 'R':
			pos++; 
			users[i].setPos(pos);
			if (pos % gridWidth == 0 && pos != 0)
			{
				PlayerDead();
				//x -= gridWidth;
				return true;
			}
			if (grid[pos].getFillColor() == sf::Color::Red)
			{
				PlayerDead();
				grid[pos].setFillColor(sf::Color::Cyan);
				return true;
			}
			else
				grid[pos].setFillColor(sf::Color::Red);
			break;
		case 'U':
			pos -= gridWidth;
			users[i].setPos(pos);
			if (pos < 0)
			{
				PlayerDead();
				//x += gridWidth*gridHeight;
				return true;
			}
			if (grid[pos].getFillColor() == sf::Color::Red)
			{
				PlayerDead();
				grid[pos].setFillColor(sf::Color::Cyan);
				return true;
			}
			else
				grid[pos].setFillColor(sf::Color::Red);
			break;
		case 'D':
			pos += gridWidth;
			users[i].setPos(pos);
			if (pos >= (gridWidth*gridHeight))
			{
				PlayerDead();
				//x -= gridWidth*gridHeight;
				return true;
			}
			if (grid[pos].getFillColor() == sf::Color::Red)
			{
				PlayerDead();
				grid[pos].setFillColor(sf::Color::Cyan);
				return true;
			}
			else
				grid[pos].setFillColor(sf::Color::Red);
			break;
		}
	}
		return false; 
}

void PlayerDead()
{
	int msgboxID = MessageBox(NULL, (LPCWSTR)L"PLAYERDEAD", (LPCWSTR)L"MESSAGE", MB_ICONWARNING);
}

void waitForValidID(ClientNetwork& CN)
{
	bool b = true;
	while (b)
	{
		if (CN.getClientNum() < 5)
		{
			b = false;
		}
	}
}

//while (stayOpen)
//{
/*sf::Event event;
while (window.pollEvent(event))
{
if (event.type == sf::Event::Closed)
window.close();
}*/