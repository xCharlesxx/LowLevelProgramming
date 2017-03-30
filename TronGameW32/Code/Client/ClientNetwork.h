#pragma once
#include <iostream>
#include <future>
#include <string>
#include <thread>
#include <SFML\Network.hpp>
#include <atomic>
using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<TcpClientPtr>;
const sf::IpAddress SERVER_IP("127.0.0.1");
constexpr int SERVER_TCP_PORT(53000);

class ClientNetwork
{
public:
	ClientNetwork();
	virtual ~ClientNetwork() = default;
	void client();
	bool connect(TcpClient& socket);
	void input(TcpClient& socket);
	void runInputThread(TcpClient & socket);
	char getCMD(); 
	int getClientNum(); 
	std::string lastMessage; 
private:
	std::atomic<int> clientNum = 9; 
	char cmd = '0'; 
	char newCmd = '0'; 
	char prevCmd = '0';
};