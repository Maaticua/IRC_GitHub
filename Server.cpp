#include "Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>

Server::Server(int port, std::string password) : _port(port), _password(password), _server_fd(-1) {}

Server::~Server()
{
	stop();
}

void Server::stop()
{
	std::map<int, Client*>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it)
		delete it->second;
	_clients.clear();

	for (size_t i = 0; i < _fds.size(); i++)
		if (_fds[i].fd != -1)
			close(_fds[i].fd);

	if (_server_fd != -1)
		close(_server_fd);
	std::cout << YELLOW << "Server Stoped" << RESET << std::endl;
}

void Server::init()
{
	int i = 1;
	struct sockaddr_in addresse;
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (_server_fd == -1)
	{
		throw std::runtime_error("Error: socket creation failed");
	}

	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) == -1)
	{
		throw std::runtime_error("Error: setsockopt");
	}

	if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		throw std::runtime_error("Error: fcntl (non-blocking)");
	}

	addresse.sin_family = AF_INET;
	addresse.sin_addr.s_addr = INADDR_ANY;
	addresse.sin_port = htons(_port);

	if (bind(_server_fd, (struct sockaddr *)&addresse, sizeof(addresse)) == -1)
	{
		throw std::runtime_error("Error: bind");
	}

	if (listen(_server_fd, 10) == -1)
	{
		throw std::runtime_error("Error: listen");
	}

	std::cout << "Server initialized on port: " << _port << std::endl;
}

void Server::run()
{
	struct pollfd server_pfd;
	server_pfd.fd = _server_fd;
	server_pfd.events = POLLIN;
	server_pfd.revents = 0;
	_fds.push_back(server_pfd);

	std::cout << "Waiting for connections..." << std::endl;

	while (true)
	{
		if (poll(&_fds[0], _fds.size(), -1) == -1)
			throw std::runtime_error("Poll failed");


		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (_fds[i].revents & POLLIN)
			{
				if (_fds[i].fd == _server_fd)
				{
					acceptNewClient();
				}
				else
				{
					handleClientMessage(i);
				}
			}
		}
	}
}

void Server::acceptNewClient()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(_server_fd, (struct sockaddr *)&client_addr, &client_len);

	if (client_fd == -1)
	{
		std::cerr << "Accept failed" << std::endl;
		return;
	}

	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(client_fd);
		return;
	}

	_clients[client_fd] = new Client(client_fd);

	struct pollfd client_pfd;
	client_pfd.fd = client_fd;
	client_pfd.events = POLLIN;
	client_pfd.revents = 0;
	_fds.push_back(client_pfd);

	std::cout << "New client connected on fd " << client_fd << std::endl;
}

void Server::handleClientMessage(size_t i)
{
	char buffer[1024];
	int fd = _fds[i].fd;
	ssize_t byte_recieved = recv(_fds[i].fd, buffer, sizeof(buffer) - 1, 0);

	if (byte_recieved <= 0)
	{
		std::cout << RED << "Client on fd " << fd << " disconected" << RESET << std::endl;
		_clientBuffer.erase(fd);
		if (_clients.count(fd))
		{
			delete _clients[fd];
			_clients.erase(fd);
		}
		close(fd);
		_fds.erase(_fds.begin() + i);
	}
	else
	{
		buffer[byte_recieved] = '\0';
		_clientBuffer[fd] += buffer;
		size_t pos;
		while ((pos = _clientBuffer[fd].find('\n')) != std::string::npos)
		{
			std::string command = _clientBuffer[fd].substr(0, pos);

			if (!command.empty() && command[command.size() - 1] == '\r')
				command.erase(command.size() - 1);
			if (!command.empty())
				std::cout << CYAN << "Commande from fd " << fd << ": " << RESET << command << std::endl;

			_clientBuffer[fd].erase(0, pos + 1);
		}
	}
}