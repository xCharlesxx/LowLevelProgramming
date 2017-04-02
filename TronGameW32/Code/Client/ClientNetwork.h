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
	void sendPacket(sf::Packet packet);
	void runInputThread(TcpClient & socket);
	char getCMD(int i); 
	int getClientNum(); 
	int UpdateNumClients();
	int requestNumClients(); 
	bool checkGameStart();
	std::string lastMessage; 
private:
	std::atomic<int> clientNum = 9; 
	std::vector<sf::Packet> packets; 
	std::mutex mtx; 
	std::atomic<int> knownClients = 0;
	std::string cmd = "0"; 
	std::string newCmd = "0"; 
	std::string prevCmd = "0";
	bool gameStart = false; 
};