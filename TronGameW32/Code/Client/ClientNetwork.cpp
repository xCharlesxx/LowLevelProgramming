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
					case '0':
					case '1':
					case '2':
					case '3':
						clientNum = (packetStore[0] - 48);
						break;
					case 'C':
						packet >> num;
						knownClients = atoi(num.c_str());
						num = ""; 
						break;
					case 'S':
						packet >> num;
						knownClients = atoi(num.c_str());
						num = "";
						gameStart = true; 
						break;
					default:
						if (packetStore[1] == clientNum)
							newCmd = packetStore[0];
						break;
					}
					if (prevCmd != newCmd)
					{
						cmd = newCmd;
						prevCmd = newCmd;
					}
				}
			}
			if (packets.size() > 10)
				int msgboxID = MessageBox(NULL, (LPCWSTR)L"Packet Overload!", (LPCWSTR)L"MESSAGE", MB_ICONWARNING);
		} while (status != sf::Socket::Disconnected);
	});
	return input(socket); 
		//runInputThread(socket);
}
//void ClientNetwork::runInputThread(TcpClient & socket)
//{
//	//std::thread input(input, socket);
//	//std::thread input(&ClientNetwork::input, this);
//}
char ClientNetwork::getCMD()
{
	return cmd;
}
int ClientNetwork::requestNumClients()
{
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
bool ClientNetwork::connect(TcpClient& socket)
{
	// attempt connection to server
	auto status = socket.connect(SERVER_IP, SERVER_TCP_PORT);
	if (status != sf::Socket::Done)
	{
		return false;
	}
	return true;
}

void ClientNetwork::input(TcpClient & socket)
{
	while (true)
	{
		//auto& sender_ref = socket;
		sf::Packet packet;
		std::string message;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			message = "L";
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			message = "R";
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			message = "U";
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			message = "D";
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			message = "S"; 
		packet << message;
		if (message != "" && message != lastMessage)
		sendPacket(packet);
		lastMessage = message; 
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
