// TronServerW32.cpp : Defines the entry point for the console application.
//

#include <Server/stdafx.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <Server/User.h>
#include <SFML\Network.hpp>

constexpr int SERVER_TCP_PORT(53000);
constexpr int SERVER_UDP_PORT(53001);

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<TcpClientPtr>;
std::vector<User> users; 
bool bindServerPort(sf::TcpListener& listener); 
void listen(sf::TcpListener & tcp_listener, sf::SocketSelector & selector, TcpClients & tcp_clients);
void connect(sf::TcpListener & tcp_listener, sf::SocketSelector & selector, TcpClients & tcp_clients);
void receiveMsg(TcpClients & tcp_clients, sf::SocketSelector & selector);
void runServer(); 
int userCount = 0; 
int main()
{
	std::cout << "Searching for life signs...\n";
	while (true)
	{
		//std::cin.get();
		runServer(); 
	}

    return 0;
}

bool bindServerPort(sf::TcpListener& listener)
{
	if (listener.listen(SERVER_TCP_PORT) != sf::Socket::Done)
	{
		std::cout << "Could not bind server port";
		std::cout << std::endl << "Port: " << SERVER_TCP_PORT;
		std::cout << std::endl;

		return false;
	}
	std::cout << "Bound to server port";
	std::cout << std::endl << "Port: " << SERVER_TCP_PORT;
	std::cout << std::endl;
	return true;
}

void listen(sf::TcpListener & tcp_listener, sf::SocketSelector & selector, TcpClients & tcp_clients)
{
	while (true)
	{
		if (selector.wait())
		{
			//new connection request
			if (selector.isReady(tcp_listener))
			{
				std::cout << "New Connection Request" << std::endl;
				connect(tcp_listener, selector, tcp_clients);
			}
			//message being recieved
			else
			{
				receiveMsg(tcp_clients, selector);
			}
		}
	}
}

void connect(sf::TcpListener & tcp_listener, sf::SocketSelector & selector, TcpClients & tcp_clients)
{
	auto client_ptr = std::make_unique<sf::TcpSocket>();
	auto& client_ref = *client_ptr;
	if (tcp_listener.accept(client_ref) == sf::Socket::Done)
	{
		User* user = new User(); 
		user->setPos(0); 
		user->setCMD("D"); 
		users.push_back(*user); 
		std::cout << "Client connected\n";
		selector.add(client_ref);
		tcp_clients.push_back(std::move(client_ptr));

		//std::cout << "Client (" << client_ptr.getClientID() << ") connected." << std::endl;
		std::string welcome_msg;
		std::string client_count = std::to_string(tcp_clients.size());
		welcome_msg += "There are " + client_count + " connected clients\n";
		std::cout << welcome_msg;
		welcome_msg = std::to_string(userCount);
		sf::Packet packet;
		packet << welcome_msg;
		client_ref.send(packet);
		userCount++; 
	}
}

void receiveMsg(TcpClients & tcp_clients, sf::SocketSelector & selector)
{
	auto sender = std::make_unique<TcpClients>();

	for (int i = 0; i < tcp_clients.size(); i++)
	{
		auto& sender_ref = *tcp_clients[i].get();
		if (selector.isReady(sender_ref))
		{
			sf::Packet packet;
			tcp_clients[i].get()->receive(packet);
			std::string string;
			std::string string1; 
			std::string string2;
			std::string string3;
			packet >> string;
			packet.clear(); 
			if (string == "C")
			{
				std::cout << "Number of Clients request Recieved from " << i << std::endl;
				packet << "C" << std::to_string(tcp_clients.size()); 
			}
			else if (string == "S")
			{
				std::cout << "Game Start request Recieved from " << i << std::endl;
				packet << "S" << std::to_string(tcp_clients.size());
			}
			else
			{
				//Add recipient ID to message and forward
				std::cout << "Message Received from " << i << std::endl;
				//packet << string << std::to_string(i);
				users[i].setCMD(string);
				for (int x = 0; x < tcp_clients.size(); x++)
				{
					packet << users[x].getCMD();
				}
			}
			std::cout << string << std::endl;
			std::cout << "Broadcasting packet " << std::endl;
			packet >> string >> string1 >> string2 >> string3; 
			std::cout << string << string1 << string2 << string3 << std::endl;
			//Broadcast to all users
			for (int i = 0; i < tcp_clients.size(); i++)
			{
				tcp_clients[i].get()->send(packet);
			}
		}
	}
}

void runServer()
{
	sf::TcpListener tcp_listener;
	if (!bindServerPort(tcp_listener))
	{
		return;
	}

	sf::SocketSelector selector;
	selector.add(tcp_listener);

	TcpClients tcp_clients;
	return listen(tcp_listener, selector, tcp_clients);
}