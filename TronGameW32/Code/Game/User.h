#pragma once
#include <Server/stdafx.h>
#include <SFML\Network.hpp>
#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
class User
{
public:
	User() = default;
	virtual ~User() = default;
	int getPos() { return pos; }
	void setPos(int newPos) { pos = newPos; }
	bool getAlive() { return isAlive; }
	void setAlive(bool _isDead) { isAlive = _isDead; }
	std::string getCMD() { return CMD; }
	void setCMD(std::string _CMD) { CMD = _CMD; }
	sf::Color getColour() { return trailColour; }
	void setColour(sf::Color colour) { trailColour = colour; }
private:
	int pos;
	std::string CMD;
	bool isAlive = false; 
	sf::Color trailColour; 
};
