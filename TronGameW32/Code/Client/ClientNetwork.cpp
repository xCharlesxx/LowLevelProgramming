#include "ClientNetwork.h"
#include <thread>
#include <SFML\Window\Keyboard.hpp>
ClientNetwork::ClientNetwork()
{
	
}

void ClientNetwork::client()
{
	TcpClient socket;
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
int ClientNetwork::getClientNum()
{
	return clientNum;
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
		auto& sender_ref = socket;
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
		packet << message;
		if (message != "" && message != lastMessage)
		socket.send(packet);
		lastMessage = message; 
	}
}
