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
	int getID() { return ID; }
	void setID(int _ID) { ID = _ID; }
private:
	int pos;
	int ID;
};
