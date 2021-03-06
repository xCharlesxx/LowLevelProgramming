// TronServerW32.cpp : Defines the entry point for the console application.
//

#include <Server/stdafx.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <Game/User.h>
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
void disconnect(TcpClients & tcp_clients);
bool receiveMsg(TcpClients & tcp_clients, sf::SocketSelector & selector);
void broadcast(TcpClients & tcp_clients, sf::Packet packet);
void runServer(); 
int user_count = 0; 
int disconnected_user = 9; 
bool flip = false; 


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
				//Recieve messages until disconnect
				if(!receiveMsg(tcp_clients, selector))
				disconnect(tcp_clients);
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
		if (flip == false)
		user->setCMD("D");
		else 
		user->setCMD("U");
		flip = !flip; 
		users.push_back(*user); 
		std::cout << "Client connected\n";
		selector.add(client_ref);
		tcp_clients.push_back(std::move(client_ptr));
		std::string welcome_msg;
		std::string client_count = std::to_string(tcp_clients.size());
		welcome_msg += "There are " + client_count + " connected clients\n";
		std::cout << welcome_msg;
		welcome_msg = std::to_string(user_count);
		sf::Packet packet;
		packet << welcome_msg;
		client_ref.send(packet);
		user_count++; 
	}
}



void disconnect(TcpClients & tcp_clients)
{
	users[disconnected_user].setCMD("X");
	std::cout << "Client Removed\n";
	tcp_clients.erase(std::remove(tcp_clients.begin(), tcp_clients.end(), tcp_clients[disconnected_user]), tcp_clients.end());
	user_count--;
}



bool receiveMsg(TcpClients & tcp_clients, sf::SocketSelector & selector)
{
	sf::Packet packet;
	std::string move = "";
	sf::Clock clock;
	for (int i = 0; i < tcp_clients.size(); i++)
	{
		auto& sender_ref = *tcp_clients[i].get();
		if (selector.isReady(sender_ref))
		{
			tcp_clients[i].get()->receive(packet);
			std::string string;
			std::string string1;
			std::string string2;
			std::string string3;
			packet >> string;
			packet >> string1;
			packet.clear();
			if (string == "")
			{
				std::cout << "Client " << i;
				std::cout << " Disconnected Unexpectedly " << std::endl;
				disconnected_user = i;
				return false;
			}
			else switch (string[0])
			{
			case 'C': 
				std::cout << "Number of Clients request Recieved from " << i << std::endl;
				packet << "C" << std::to_string(tcp_clients.size());
				broadcast(tcp_clients, packet);
				break;
			case 'S':
				std::cout << "Game Start request Recieved from " << i << std::endl;
				packet << "S" << std::to_string(tcp_clients.size());
				broadcast(tcp_clients, packet);
				break;
			case 'X':
				std::cout << "player Died Recieved From " << i << std::endl;
				users[atoi(string1.c_str())].setCMD("X");
				break;
			case 'K':
				std::cout << "Client " << i;
				std::cout << " Disconnected Gracefully " << std::endl;
				disconnected_user = i; 
				disconnect(tcp_clients);
				packet << "C" << std::to_string(tcp_clients.size());
				broadcast(tcp_clients, packet);
				break;
			case 'G':
				std::cout << "Client " << i;
				std::cout << " Requested Reset " << std::endl;
				packet << string;
				broadcast(tcp_clients, packet); 
				for (int x = 0; x < users.size(); x++)
				{
					if (i == 1 || i == 3)
						users[i].setCMD("U");
					else
						users[i].setCMD("D"); 
				}
			case 'P':
				users[i].setPos(1);
				for (int i = 0; i < users.size(); i++)
				{
					if (users[i].getPos() != 1)
					{
						break;
					}
				}
				//if all players have sent in a ping 
				move = "";
				for (int x = 0; x < tcp_clients.size(); x++)
				{
					move += users[x].getCMD();
				}
				clock.restart(); 
				packet << move;
				broadcast(tcp_clients, packet); 
				for (int i = 0; i < users.size(); i++)
					users[i].setPos(0); 
				break;
			default:
				//Update CMD to message
				users[i].setCMD(string);
				break;
			}
			std::cout << "Message Received from " << i << std::endl;
			std::cout << string << std::endl;
			std::cout << "Broadcasting packet " << std::endl;
			packet >> string >> string1 >> string2 >> string3;
			std::cout << string << string1 << string2 << string3 << std::endl;
		}
	}
	return true; 
}



void broadcast(TcpClients & tcp_clients, sf::Packet packet)
{
	//Send message to Clients
	for (int i = 0; i < tcp_clients.size(); i++)
	{
		tcp_clients[i].get()->send(packet);
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