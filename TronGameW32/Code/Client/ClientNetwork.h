#pragma once
#include <iostream>
#include <future>
#include <string>
#include <thread>
#include <SFML\Network.hpp>
#include <atomic>
#include "Game\User.h"
using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<TcpClientPtr>;
const sf::IpAddress SERVER_IP("127.0.0.1");
constexpr int SERVER_TCP_PORT(53000);

class ClientNetwork
{
public:
	ClientNetwork() = default;
	virtual ~ClientNetwork() = default;
	void client();
	bool connect(TcpClient& socket);
	void disconnect(); 
	void input(TcpClient& socket);
	void sendPacket(sf::Packet packet);
	const char getCMD(const int i); 
	void clearCMD(); 
	const int getClientNum(); 
	const int updateNumClients();
	const int requestNumClients(); 
	const bool checkGameStart();
	const bool heartBeatPlayer(const int i); 
	const bool readyToGo(); 
private:
	std::atomic<int> client_num = 9; 
	std::vector<sf::Packet> packets; 
	std::vector<User> users; 
	std::mutex mtx; 
	std::atomic<int> known_clients = 0;
	std::string cmd = "0000"; 
	std::string prev_msg = ""; 
	bool game_start = false; 
	bool has_disconnected = false; 
	bool axis_of_movement = false;
	bool players_are_set = false; 
};