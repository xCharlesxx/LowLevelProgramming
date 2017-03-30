#include <SFML/Graphics.hpp>
#include <iostream>
#include <future>
#include <string>
#include <SFML\Network.hpp>
#include "ClientNetwork.h"
#include <Windows.h>
int x = 0; 
void PlayerDead(); 
void waitForValidID(ClientNetwork& CN); 
int main()
{
	ClientNetwork* CN = new ClientNetwork(); 
	sf::RenderWindow window(sf::VideoMode(600, 600), "SFML works!");
	std::vector<sf::CircleShape> grid; 
	int gridWidth = 30;
	int gridHeight = 30;
	grid.reserve(gridWidth*gridHeight); 
	float shapeSize = 10.0f; 
	sf::Clock clock; // starts the clock
	for (int i = 0; i < gridWidth*gridHeight; i++)
	{
		int x = i % gridWidth; 
		int y = i / gridWidth;

		sf::CircleShape shape(shapeSize);
		shape.setFillColor(sf::Color::Green);
		shape.setPosition(((shapeSize*2)*x), ((shapeSize * 2)*y));
		grid.push_back(shape); 
	}

	std::thread networking(&ClientNetwork::client, CN);
	waitForValidID(*CN);
	x = CN->getClientNum() * 10; 
	grid[x].setFillColor(sf::Color::Blue);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		window.clear();
		
		if (clock.getElapsedTime().asSeconds() > 0.1)
		{
			switch (CN->getCMD())
			{
			case 'L':
				if (x < 0 || x % gridWidth == 0)
				{
					PlayerDead();
					x += gridWidth;
				}
				x--;
				if (grid[x].getFillColor() == sf::Color::Red)
				{
					PlayerDead();
					grid[x].setFillColor(sf::Color::Cyan);
				}
				else
					grid[x].setFillColor(sf::Color::Red); 
				break;
			case 'R':
				x++;
				if (x % gridWidth == 0 && x != 0)
				{
					PlayerDead();
					x -= gridWidth;
				}
				if (grid[x].getFillColor() == sf::Color::Red)
				{
					PlayerDead();
					grid[x].setFillColor(sf::Color::Cyan);
				}
				else
					grid[x].setFillColor(sf::Color::Red);
				break;
			case 'U':
				x -= gridWidth;
				if (x < 0)
				{
					PlayerDead();
					x += gridWidth*gridHeight;
				}
				if (grid[x].getFillColor() == sf::Color::Red)
				{
					PlayerDead();
					grid[x].setFillColor(sf::Color::Cyan);
				}
				else
					grid[x].setFillColor(sf::Color::Red);
				break;
			case 'D':
				x += gridWidth;
				if (x >= (gridWidth*gridHeight))
				{
					PlayerDead(); 
					x -= gridWidth*gridHeight; 
				}
				if (grid[x].getFillColor() == sf::Color::Red)
				{
					PlayerDead();
					grid[x].setFillColor(sf::Color::Cyan);
				}
				else
					grid[x].setFillColor(sf::Color::Red);
				break;
			}
			clock.restart(); 
		}
		for (int i = 0; i < grid.size(); i++)
		{
			window.draw(grid[i]);
		}
		window.display();
	}

	return 0;
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