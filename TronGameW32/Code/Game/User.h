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
	const int getPos()                     { return pos; }
	const bool getAlive()                  { return is_alive; }
	const std::string getCMD()             { return CMD; }
	const sf::Color getColour()            { return trail_colour; }
	void setPos(const int newPos)          { pos = newPos; }
	void setAlive(const bool _isDead)      { is_alive = _isDead; }
	void setCMD(const std::string _CMD)    { CMD = _CMD; }
	void setColour(const sf::Color colour) { trail_colour = colour; }
private:
	int pos;
	std::string CMD;
	bool is_alive = false; 
	sf::Color trail_colour; 
};
