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
	char getCMD() { return CMD; }
	void setCMD(char _CMD) { CMD = _CMD; }
private:
	int pos;
	char CMD;
};
