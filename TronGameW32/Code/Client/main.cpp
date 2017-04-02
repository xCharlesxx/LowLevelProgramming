#include <SFML/Graphics.hpp>
#include <iostream>
#include <future>
#include <string>
#include <SFML\Network.hpp>
#include "ClientNetwork.h"
#include "Game\User.h"
#include <Windows.h>

enum class MainMenuSelection
{
	INVALID = -1,
	NONE = 0,
	JOIN_GAME,
	QUIT
};
enum class SceneSelector
{
	INVALID = -1,
	NONE = 0,
	MENU,
	LOBBY,
	GAME
};
void PlayerDead(std::vector<User> &users, const int i);
void waitForValidID(ClientNetwork& CN); 
bool MainMenu(MainMenuSelection &MMS, sf::Sprite &JoinGame);
bool Game(ClientNetwork& CN, std::vector<sf::CircleShape>& grid, const int gridWidth, const int gridHeight, sf::Clock clock, std::vector<User> &users);
bool lobby(ClientNetwork& CN);
void init(ClientNetwork& CN, SceneSelector &SS, sf::Sprite &JoinGame, std::vector<sf::CircleShape> &players, int window_x, int window_y, std::vector<sf::CircleShape> &grid, std::vector<User> &users);
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
	sf::Clock clock;
	std::vector<sf::CircleShape> players;
	SceneSelector SS; 
	SS = SceneSelector::MENU; 
	JoinGame.setTexture(t_Start);
	ClientNetwork* CN = new ClientNetwork();
	std::vector<User> users;
	std::vector<sf::CircleShape> grid;
	init(*CN, SS, JoinGame, players, 0, 0, grid, users);

	while (window.isOpen())
	{
		window.clear();
		switch (SS)
		{
		case SceneSelector::INVALID:
			break;
		case SceneSelector::NONE:
			break;
		case SceneSelector::MENU:
			if (MainMenu(MMS, JoinGame) == true)
			{
				if (MMS == MainMenuSelection::QUIT)
				{
					window.close();
					break; 
				}
				SS = SceneSelector::LOBBY;
				new std::thread(&ClientNetwork::client, CN);
				waitForValidID(*CN);
				JoinGame.setTexture(t_Lobby);
				init(*CN, SS, JoinGame, players, window.getSize().x, window.getSize().y, grid, users);
				CN->UpdateNumClients();
			}
			window.draw(JoinGame);
			break;
		case SceneSelector::LOBBY:
			//Check lobby contents every two seconds
			if (clock.getElapsedTime().asSeconds() > 2)
			{
				for (int i = 0; i < CN->requestNumClients(); i++)
				{
					players[i].setFillColor(sf::Color::Green);
				}
				clock.restart();
			}
			//Check if game has been started
			if (lobby(*CN) == true)
			{
				SS = SceneSelector::GAME;
				CN->requestNumClients();
				JoinGame.setTexture(t_Lobby);
				init(*CN, SS, JoinGame, players, 0, 0, grid, users);
			}
			for (int i = 0; i < 4; i++)
			{
				window.draw(players[i]);
			}
			window.draw(JoinGame);
			break;
		case SceneSelector::GAME:
			Game(*CN, grid, 30, 30, clock, users);
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
	case SceneSelector::MENU:
		JoinGame.setColor(sf::Color::Green);
		JoinGame.setPosition(150, 100);
		break;
	case SceneSelector::LOBBY:
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
		grid.reserve(gridWidth*gridHeight);
		for (int i = 0; i < gridWidth*gridHeight; i++)
		{
			int x = i % gridWidth;
			int y = i / gridWidth;
			sf::CircleShape shape(shapeSize);
			shape.setFillColor(sf::Color::Green);
			shape.setPosition(((shapeSize * 2)*x), ((shapeSize * 2)*y));
			grid.push_back(shape);
		}
		users.reserve(4);
		for (int i = 0; i < CN.requestNumClients(); i++)
		{
			User* user = new User();
			user->setPos(i * (gridWidth*gridHeight / 4));
			user->setAlive(true); 
			users.push_back(*user);
			grid[users[i].getPos()].setFillColor(sf::Color::Blue);
		}
		break;
	default:
		break;
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
	std::string dead = "X";
	sf::Packet packet; 
	for (int i = 0; i < users.size(); i++)
	{
		if (!CN.heartBeatPlayer(i))
			users[i].setAlive(false); 
	}
	while (!(clock.getElapsedTime().asSeconds() > 0.1)) {}
	for (int i = 0; i < users.size(); i++)
	{
		if (users[i].getAlive() == true)
		{
			int pos = users[i].getPos();
			switch (CN.getCMD(i))
			{
			case 'L':
				if (pos % gridWidth == 0)
				{
					PlayerDead(users, i);
					packet << dead; 
					dead = std::to_string(i);
					packet << dead; 
					CN.sendPacket(packet);
					return true;
				}
				pos--;
				users[i].setPos(pos);
				if (grid[pos].getFillColor() == sf::Color::Red)
				{
					PlayerDead(users, i);
					grid[pos].setFillColor(sf::Color::Cyan);
					return true;
				}
				else
					grid[pos].setFillColor(sf::Color::Red);
				break;
			case 'R':
				pos++;
				users[i].setPos(pos);
				if (pos % gridWidth == 0)
				{
					PlayerDead(users, i);
					//x -= gridWidth;
					return true;
				}
				if (grid[pos].getFillColor() == sf::Color::Red)
				{
					PlayerDead(users, i);
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
					PlayerDead(users, i);
					//x += gridWidth*gridHeight;
					return true;
				}
				if (grid[pos].getFillColor() == sf::Color::Red)
				{
					PlayerDead(users, i);
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
					PlayerDead(users, i);
					//x -= gridWidth*gridHeight;
					return true;
				}
				if (grid[pos].getFillColor() == sf::Color::Red)
				{
					PlayerDead(users, i);
					grid[pos].setFillColor(sf::Color::Cyan);
					return true;
				}
				else
					grid[pos].setFillColor(sf::Color::Red);
				break;
			}
		}
	}
		return false; 
}

void PlayerDead(std::vector<User> &users, const int i)
{
	users[i].setAlive(false); 
	//int msgboxID = MessageBox(NULL, (LPCWSTR)L"PLAYERDEAD", (LPCWSTR)L"MESSAGE", MB_ICONWARNING);
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