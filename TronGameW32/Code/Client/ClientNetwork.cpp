#include "ClientNetwork.h"
#include <thread>
#include <Windows.h>
#include <SFML\Window\Keyboard.hpp>
ClientNetwork::ClientNetwork()
{
	
}

void ClientNetwork::client()
{
	TcpClient socket;
	packets.reserve(10); 
	if (!connect(socket))
	{
		return;
	}
	users.reserve(4); 

	auto handle = std::async(std::launch::async, [&]
	{
		// keep track of the socket status
		sf::Socket::Status status;
		do
		{
			sf::Packet packet;
			status = socket.receive(packet);
			if (status == sf::Socket::Done)
			{
				std::string num;
				std::string packetStore;
				packet >> packetStore; 
				if (packetStore != "")
				{
					switch (packetStore[0])
					{
						//Get ID
					case '0':
					case '1':
					case '2':
					case '3':
						//Convert from ascii
						clientNum = (packetStore[0] - 48);
						break;
						//Reply for number of clients
					case 'C':
						packet >> num;
						knownClients = atoi(num.c_str());
						num = ""; 
						break;
						//Start Game command
					case 'S':
						packet >> num;
						knownClients = atoi(num.c_str());
						num = "";
						gameStart = true; 
						break;
						//PlayerDead update
					case 'X':
						packet >> num; 
						users[atoi(num.c_str())].setAlive(false);
						num = "";
						break;
						//Reset game
					case 'G':
						gameStart = false; 
						axisOfMovement = false; 
						break;
						//Standard player movement update
					default:
						cmd = packetStore;
						break;
					}
					for (int i = 0; i < users.size(); i++)
					{
						if (packetStore[i] == 'X')
							users[i].setAlive(false); 
					}
					if (hasDisconnected == true)
						socket.disconnect();
				}
			}
			if (packets.size() > 10)
				int msgboxID = MessageBox(NULL, (LPCWSTR)L"Packet Overload!", (LPCWSTR)L"MESSAGE", MB_ICONWARNING);
		} while (status != sf::Socket::Disconnected);
	});
	return input(socket); 
}

char ClientNetwork::getCMD(int i)
{
	return cmd[i];
}
void ClientNetwork::clearCMD()
{
	cmd = "0000"; 
}
int ClientNetwork::requestNumClients()
{
	users.clear(); 
	for (int i = 0; i < knownClients; i++)
	{
		User* user = new User();
		user->setAlive(true);
		users.push_back(*user); 
	}
	return knownClients;
}
int ClientNetwork::UpdateNumClients()
{
	knownClients = 0;
	sf::Packet packet;
	const std::string msg = "C";
	packet << msg;
	sendPacket(packet);
	while (knownClients == 0) {}
	return knownClients;
}
int ClientNetwork::getClientNum()
{
	return clientNum;
}
bool ClientNetwork::checkGameStart()
{
	return gameStart; 
}
bool ClientNetwork::heartBeatPlayer(int i)
{
	return users[i].getAlive();
}
bool ClientNetwork::connect(TcpClient& socket)
{
	// attempt connection to server
	auto status = socket.connect(SERVER_IP, SERVER_TCP_PORT);
	if (status != sf::Socket::Done)
	{
		return false;
	}
	hasDisconnected = false; 
	return true;
}
void ClientNetwork::disconnect()
{
	sf::Packet packet;
	std::string temp = "K"; 
	packet << temp; 
	sendPacket(packet); 
	hasDisconnected = true; 
}
void ClientNetwork::input(TcpClient & socket)
{

	while (getClientNum() > 5) {}
	if (sf::Joystick::isConnected(getClientNum()))
	{
		int sensitivity = 50; 
		while (true)
		{
			sf::Packet packet;
			std::string message;
			float x = sf::Joystick::getAxisPosition(getClientNum(), sf::Joystick::X);
			float y = sf::Joystick::getAxisPosition(getClientNum(), sf::Joystick::Y);
			if (axisOfMovement == true)
			{
				if (y > sensitivity)
					message = "D";
				if (y < -sensitivity)
					message = "U";
			}
			else
			{
				if (x > sensitivity)
					message = "R";
				if (x < -sensitivity)
					message = "L";
			}
			if (sf::Joystick::isButtonPressed(getClientNum(), 0) && gameStart != true)
				message = "S";
			if (message != "")
			{
				packet << message;
				sendPacket(packet);
				if (message != "S")
				axisOfMovement = !axisOfMovement; 
			}
			while (!mtx.try_lock()) {};
			for (int i = 0; i < packets.size(); i++)
				socket.send(packets[i]);
			packets.clear();
			mtx.unlock();
		}
	}
	while (true)
	{
		sf::Packet packet;
		std::string message;
		if (axisOfMovement == true)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
				message = "U";
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
				message = "D";
		}
		else
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
				message = "L";
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
				message = "R";
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && gameStart != true)
			message = "S";
		if (message != "" && message != prevMessage)
		{
			packet << message;
			sendPacket(packet);
			if (message != "S")
			axisOfMovement = !axisOfMovement;
			prevMessage = message; 
		}
		while (!mtx.try_lock()) {};
		for (int i = 0; i < packets.size(); i++)
			socket.send(packets[i]);
		packets.clear();
		mtx.unlock();
	}
}

void ClientNetwork::sendPacket(sf::Packet packet)
{
	mtx.lock(); 
	std::string contents; 
	packet >> contents;
	if (contents != "")
	{
		packet << contents;
		packets.push_back(packet);
	}
	mtx.unlock(); 
}



//if (getClientNum() == 1)
//{
//	while (true)
//	{
//		//auto& sender_ref = socket;
//		sf::Packet packet;
//		std::string message;
//		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
//			message = "L";
//		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
//			message = "R";
//		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
//			message = "U";
//		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
//			message = "D";
//		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
//			message = "S";
//		packet << message;
//		if (message != "" && message != lastMessage)
//			sendPacket(packet);
//		lastMessage = message;
//		while (!mtx.try_lock()) {};
//		for (int i = 0; i < packets.size(); i++)
//			socket.send(packets[i]);
//		packets.clear();
//		mtx.unlock();
//	}
//}
//else
//{