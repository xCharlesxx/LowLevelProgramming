#pragma once
#include <Server/stdafx.h>
#include <SFML\Network.hpp>
#include <iostream>
#include <memory>

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
class User
{
public:
	User() = default;
	virtual ~User() = default;
	int getPos() { return pos; }
	void setPos(int newPos) { pos = newPos; }
	std::string getCMD() { return CMD; }
	void setCMD(std::string _CMD) { CMD = _CMD; }
private:
	int pos;
	std::string CMD;
};
