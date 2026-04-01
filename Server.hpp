#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Colors.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <poll.h>

class Server
{
	private:
		int _port;
		std::string _password;
		int _server_fd;
		std::map<int, Client*> _clients;
		std::map<int, std::string> _clientBuffer;
		std::vector<struct pollfd> _fds;
		Server(const Server &src);
		Server &operator=(const Server &src);

	public:
		Server(int port, std::string password);
		~Server();

		void init();
		void run();
		void stop();
		void acceptNewClient();
		void handleClientMessage(size_t i);
		void processCommand(int fd, std::string command);
};

#endif