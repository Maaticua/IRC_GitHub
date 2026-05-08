#include "Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstdlib>


extern bool g_running;

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

	std::map<std::string, Channel*>::iterator itChan;
	for (itChan = _channels.begin(); itChan != _channels.end(); ++itChan)
		delete itChan->second;
	_channels.clear();

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

	while (g_running)
	{
		if (poll(&_fds[0], _fds.size(), -1) == -1)
		{
			if (!g_running)
				break;
			throw std::runtime_error("Poll failed");
		}


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
		if (_clients.count(fd))
		{
			leaveAllChannel(_clients[fd]);
			_clientBuffer.erase(fd);
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
				processCommand(fd, command);

			_clientBuffer[fd].erase(0, pos + 1);
		}
	}
}

void Server::processCommand(int fd, std::string commande)
{
	std::stringstream ss(commande);
	std::string cmdName;
	ss >> cmdName;

	Client *client = _clients[fd];

	if (cmdName == "PASS")
	{
		std::string check;
		ss >> check;
		if (check == _password)
			client->has_pass = true;
		else
			sendResponse(fd, "464 * : Password incorect");
	}
	else if (!client->has_pass)
	{
		sendResponse(fd, "451 * : Not provided the good password");
		return;
	}
	else if (cmdName == "NICK")
	{
		std::string nick;
		ss >> nick;
		if (nick.empty())
			sendResponse(fd, "431 * : No nickname");
		else
		{
			bool inUse = false;
			std::map<int, Client*>::iterator it;
			for (it = _clients.begin(); it != _clients.end(); ++it)
			{
				if (it->second->nickname == nick && it->second->fd != fd)
				{
					inUse = true;
					break;
				}
			}
			if (inUse)
			{
				sendResponse(fd, "433 * " + nick + " :Nickename is already in use");
			}
			else
			{
				client->nickname = nick;
				std::cout << "Client fd " << fd << " is known as " << nick << std::endl;
			}
		}
	}
	else if (cmdName == "USER")
	{
		std::string user, host, server, real;
		ss >> user >> host >> server;
		std::getline(ss, real);

		if (user.empty() || real.empty())
			sendResponse(fd, "461 * USER : Not enough parameters");
		else
		{
			client->username = user;
			if (!client->nickname.empty() && !client->is_registered)
			{
				client->is_registered = true;
				sendResponse(fd, "001 " + client->nickname + " : Welcome to the IRC server");
			}
		}
	}
	else if (cmdName == "JOIN")
	{
		std::string chanName, key;
		ss >> chanName >> key; // on chope le channel et le mdp

		if (!client->is_registered)
		{
			sendResponse(fd, "451 *:You have not registered");
			return;
		}

		if (chanName.empty() || chanName[0] != '#')
		{
			sendResponse(fd, "403 " + client->nickname + " " + chanName + " :No channel");
			return;
		}

		if (_channels.find(chanName) == _channels.end())
		{
			_channels[chanName] = new Channel(chanName);
			_channels[chanName]->operators.push_back(client);
			std::cout << CYAN << "Channel " << chanName << " created by " << client->nickname << RESET << std::endl;
		}
		else
		{
			Channel *chan = _channels[chanName];

			if (chan->max_user > 0 && chan->members.size() >= (size_t)chan->max_user) // check de la limite user
			{
				sendResponse(fd, "471 " + client->nickname + " " + chanName + " :Cannot join channel (+1)");
				return;
			}

			if (!chan->key.empty() && chan->key != key) // check du mdp
			{
				sendResponse(fd, "475 " + client->nickname + " " + chanName + " :Cannot join channel (+k)");
				return;
			}

			if (chan->mode_i) // check de l'invite
			{
				bool isInvited = false;
				for (size_t j = 0; j < chan->invited_user.size(); j++)
				{
					if (chan->invited_user[j] == client->nickname)
					{
						isInvited = true;
						break;
					}
				}
				if (!isInvited)
				{
					sendResponse(fd, "473 " + client->nickname + " " + chanName + " :Cannot join channel (+i)");
					return;
				}
			}
		}
		Channel *chan = _channels[chanName];
		bool isMember = false;
		for (size_t i = 0; i < chan->members.size(); i++)
			if (chan->members[i]->fd == fd)
				isMember = true;

		if (!isMember)
		{
			chan->members.push_back(client);
			std::string joinMsg = ":" + client->nickname + "!" + client->username + "@localhost JOIN :" + chanName;
			chan->broadcast(joinMsg, -1);
		}
	}

	else if (cmdName == "PRIVMSG")
	{
		std::string target, text;
		ss >> target;
		std::getline(ss, text);

		if (!client->is_registered)
		{
			sendResponse(fd, "451 * :You have not registered");
			return;
		}
		if (!text.empty() && text[1] == ':')
			text = text.substr(2);
		else if (!text.empty())
			text = text.substr(1);

		if (target.empty() || text.empty())
		{
			sendResponse(fd, "411 " + client->nickname + " :No recipient given");
			return;
		}
		if (target[0] == '#')
		{
			if (_channels.find(target) == _channels.end())
				sendResponse(fd, "403 " + client->nickname + " " + target + ":No such channel");
			else
			{
				std::string formattedMsg = ":" + client->nickname + "!" + client->username + "@localhost PRIVMSG " + target + " :" + text;
				_channels[target]->broadcast(formattedMsg, fd);
			}
		}
		else
		{
			int targetFd = -1;
			std::map<int, Client*>::iterator it;
			for (it = _clients.begin(); it != _clients.end(); it++)
			{
				if (it->second->nickname == target)
				{
					targetFd = it->first;
					break;
				}
			}
			if (targetFd != -1)
			{
				std::string formattedMsg = ":" + client->nickname + "!" + client->username + "@localhost PRIVMSG " + target + " :" + text;
				sendResponse(targetFd, formattedMsg);
			}
			else
				sendResponse(fd, "401 " + client->nickname + " " + target + " :No such nick/channel");
		}
	}
	else if (cmdName == "KICK")
	{
		std::string chanName, targetNick, reason;
		ss >> chanName >> targetNick;
		std::getline(ss, reason);

		if (!reason.empty() && reason[0] == ' ')
			reason.erase(0, 1);
		if (!reason.empty() && reason[0] == ':')
			reason.erase(0, 1);
		if (reason.empty())
			reason = "Kicked by operator";

		if (_channels.find(chanName) == _channels.end())
		{
			sendResponse(fd, "403 " + client->nickname + " " + chanName + " :No such channel");
			return;
		}
		Channel *chan = _channels[chanName];

		bool isOP = false;
		for (size_t i = 0; i < chan->operators.size(); i++)
		{
			if (chan->operators[i]->fd == fd)
			{
				isOP = true;
				break;
			}
		}

		if (!isOP)
		{
			sendResponse(fd, "482 " + client->nickname + " " + chanName + " :You are not channel operator");
			return;
		}

		Client *targetClient = NULL;
		std::vector<Client*>::iterator it;
		for (it = chan->members.begin(); it != chan->members.end(); ++it)
		{
			if ((*it)->nickname == targetNick)
			{
				targetClient = *it;
				break;
			}
		}

		if (!targetClient)
		{
			sendResponse(fd, "441 " + targetNick + " " + chanName + " :They are not on that channel");
			return;
		}

		std::string kickClient = ":" + client->nickname + "!" + client->username + "@localhost KICK " + chanName + " " + targetNick + " :" + reason;
		chan->broadcast(kickClient, -1);

		chan->members.erase(it);

		for (std::vector<Client*>::iterator oit = chan->operators.begin(); oit != chan->operators.end(); ++oit)
		{
			if ((*oit)->fd == targetClient->fd)
			{
				chan->operators.erase(oit);
				break;
			}
		}
	}
	else if (cmdName == "TOPIC")
	{
		std::string chanName, newTopic;
		ss >> chanName;
		std::getline(ss, newTopic);

		if (_channels.find(chanName) == _channels.end())
		{
			sendResponse(fd, "403 " + client->nickname + " " + chanName + " : No such channel");
			return;
		}

		Channel *chan = _channels[chanName];
		bool isMember = false;

		for (size_t i = 0; i < chan->members.size(); i++)
			if (chan->members[i]->fd == fd)
				isMember = true;

		if (!isMember)
		{
			sendResponse(fd, "442 " + client->nickname + " " + chanName + " :You're not on that channel");
			return;
		}

		if (newTopic.empty())
		{
			if (chan->topic.empty())
				sendResponse(fd, "331 " + client->nickname + " " + chanName + " : No topic is set");
			else
				sendResponse(fd, "332 " + client->nickname + " " + chanName + " :" + chan->topic);
		}
		else
		{
			if (chan->mode_t)
			{
				bool isOP = false;
				for (size_t j = 0; j < chan->operators.size(); j++)
				{
					if (chan->operators[j]->fd == fd)
					{
						isOP = true;
						break;
					}
				}
				if (!isOP)
				{
					sendResponse(fd, "482 " + client->nickname + " " + chanName + " :You're not channel operator");
					return;
				}
			}
			if (newTopic[1] == ':')
				newTopic = newTopic.substr(2);
			else
				newTopic = newTopic.substr(1);
			chan->topic = newTopic;
			std::string msg = ":" + client->nickname + "!" + client->username + "@localhost TOPIC " + chanName + " :" + chan->topic;
			chan->broadcast(msg, -1);
		}
	}
	else if (cmdName == "INVITE")
	{
		std::string targetNick, chanName;
		ss >> targetNick >> chanName;

		if (targetNick.empty() || chanName.empty())
		{
			sendResponse(fd, "461 " + client->nickname + " INVITE :Not enough parameters");
			return;
		}

		if (_channels.find(chanName) == _channels.end())
		{
			sendResponse(fd, "403 " + client->nickname + " " + chanName + " :No such channel");
			return;
		}

		Channel *chan = _channels[chanName];
		bool isMember = false;
		for (size_t i = 0; i < chan->members.size(); i++)
			if (chan->members[i]->fd == fd) isMember = true;

		if (!isMember)
		{
			sendResponse(fd, "442 " + client->nickname + " " + chanName + " :You're not on that channel");
			return;
		}

		Client *targetClient = NULL;
		std::map<int, Client*>::iterator it;
		for (it = _clients.begin(); it != _clients.end(); ++it)
		{
			if (it->second->nickname == targetNick)
			{
				targetClient = it->second;
				break;
			}
		}

		if (!targetClient)
		{
			sendResponse(fd, "401 " + client->nickname + " " + targetNick + " :No such nickname");
			return;
		}

		for (size_t i = 0; i < chan->members.size(); i++)
		{
			if (chan->members[i]->nickname == targetNick)
			{
				sendResponse(fd, "443 " + client->nickname + " " + targetNick + " " + chanName + " :is already on channel");
				return;
			}
		}

		chan->invited_user.push_back(targetNick);
		std::string inviteMsg = ":" + client->nickname + "!" + client->username + "@localhost INVITE " + targetNick + " :" + chanName;
		sendResponse(targetClient->fd, inviteMsg);

		sendResponse(fd, "341 " + client->nickname + " " + targetNick + " " + chanName);

	}
	else if (cmdName == "MODE")
	{
		std::string target, flags, param;
		ss >> target >> flags;

		if (target[0] != '#')
			return;

		if (_channels.find(target) == _channels.end())
		{
			sendResponse(fd, "403 " + client->nickname + " " + target + " :No such channel");
			return;
		}

		Channel *chan = _channels[target];
		bool isOP = false;

		for (size_t i = 0; i < chan->operators.size(); i++)
			if (chan->operators[i]->fd == fd)
				isOP = true;

		if (!isOP)
		{
			sendResponse(fd, "482 " + client->nickname + " " + target + " : You are not channel operator");
			return;
		}

		if (flags.empty())
			return ;

		bool add = true; // on part du principe qu'on ajoute

		std::string appliedModes = "";
		std::string appliedParams = "";

		for (size_t i = 0; i < flags.size(); i++)
		{
			char c = flags[i];

			if (c == '+')
			{
				add = true;
				appliedModes += "+";
			}
			else if (c == '-')
			{
				add = false;
				appliedModes += "-";
			}
			else if (c == 'i')
			{
				chan->mode_i = add; // true si +, false si -
				appliedModes += "i";
			}
			else if (c == 't')
			{
				chan->mode_t = add;
				appliedModes += "t";
			}
			else if (c == 'k')
			{
				if (add)
				{
					if (ss >> param) // on essaie de lire le mdp
					{
						chan->key = param;
						appliedModes += "k";
						appliedParams += " " + param;
					}
				}
				else
				{
					chan->key = ""; //si on retire on efface le mdp
					appliedModes += "k";
				}
			}
			else if (c == 'l')
			{
				if (add)
				{
					if (ss >> param) // on essaie de lire la limite
					{
						chan->max_user = atoi(param.c_str());
						appliedModes += "l";
						appliedParams += " " + param;
					}
				}
				else
				{
					chan->max_user = 0;
					appliedModes += "l";
				}
			}
			else if (c == 'o')
			{
				if (ss >> param) // on a besoin du pseudo
				{
					Client* targetClient = NULL;

					for (size_t j = 0; j < chan->members.size(); j++)
					{
						if (chan->members[j]->nickname == param)
						{
							targetClient = chan->members[j];
							break;
						}
					}
					if (targetClient)
					{
						if (add)
						{
							bool alreadyOp = false;
							for (size_t j = 0; j < chan->operators.size(); j++)
							{
								if (chan->operators[j]->fd == targetClient->fd)
									alreadyOp = true;
							}
							if (!alreadyOp)
							{
								chan->operators.push_back(targetClient);
								appliedModes += "o";
								appliedParams += " " + param;
							}
						}
						else
						{
							for (std::vector<Client*>::iterator it = chan->operators.begin(); it != chan->operators.end(); it++)
							{
								if ((*it)->fd == targetClient->fd)
								{
									chan->operators.erase(it);
									appliedModes += "o";
									appliedParams += " " + param;
									break;
								}
							}
						}
					}
				}
			}
		}
		if (appliedModes != "+" && appliedModes != "-" && !appliedModes.empty()) // si on a au moins un mode on l'annonce
		{
			std::string modeNotif = ":" + client->nickname + "!" + client->username + "@localhost MODE " + target + " " + appliedModes + appliedParams;
			chan->broadcast(modeNotif, -1);
		}
	}
	else if (cmdName == "PING")
	{
		std::string param;
		ss >> param;
		sendResponse(fd, "PONG " + param);
		return;
	}
	else if (cmdName == "PART")
	{
		std::string chanName, reason;
		ss >> chanName;
		std::getline(ss, reason);

		if (!reason.empty() && reason[0] == ' ')
			reason.erase(0, 1);
		if (!reason.empty() && reason[0] == ':')
			reason.erase(0, 1);
		if (reason.empty())
			reason = "Leaving";

		if (_channels.find(chanName) == _channels.end())
		{
			sendResponse(fd, "403 " + client->nickname + " " + chanName + " :No such channel");
			return;
		}

		Channel *chan = _channels[chanName];
		bool isMember = false;

		for (std::vector<Client*>::iterator it = chan->members.begin(); it != chan->members.end(); ++it) // on cherche le client
		{
			if ((*it)->fd == fd)
			{
				isMember = true;

				std::string partMsg = ":" + client->nickname + "!" + client->username + "@localhost PART " + chanName + " :" + reason;
				chan->broadcast(partMsg, -1);
				chan->members.erase(it);
				break;
			}
		}

		if (!isMember)
		{
			sendResponse(fd, "442 " + client->nickname + " " + chanName + " :You're not on that channel");
			return;
		}

		// si il etait operateur on lui enleve ses privileges
		for (std::vector<Client*>::iterator oit = chan->operators.begin(); oit != chan->operators.end(); ++oit)
		{
			if ((*oit)->fd == fd)
			{
				chan->operators.erase(oit);
				break;
			}
		}

		if (chan->members.empty()) // si ya plus personne on efface le channel
		{
			delete chan;
			_channels.erase(chanName);
		}

	}
	else if (cmdName == "QUIT")
	{
		std::string reason;
		std::getline(ss, reason);
		std::cout << "Client " << client->nickname << " wants to quit (" << reason << ")" << std::endl;
	}
}

void Server::sendResponse(int fd, std::string reponse)
{
	std::string msg = reponse + "\r\n";
	if (send(fd, msg.c_str(), msg.length(),0) == -1)
		std::cerr << RED << "Error: send fail" << RESET << std::endl;
}

void Server::leaveAllChannel(Client *client)
{
	std::map<std::string, Channel*>::iterator it = _channels.begin();

	while (it != _channels.end())
	{
		Channel *chan = it->second;
		bool found = false;

		for (std::vector<Client*>::iterator mit = chan->members.begin(); mit != chan->members.end(); ++mit)
		{
			if ((*mit)->fd == client->fd)
			{
				chan->members.erase(mit);
				found = true;
				break;
			}
		}

		for (std::vector<Client*>::iterator oit = chan->operators.begin(); oit != chan->operators.end(); ++oit)
		{
			if ((*oit)->fd == client->fd)
			{
				chan->operators.erase(oit);
				break;
			}
		}

		if (found)
		{
			std::string partMsg = ":" + client->nickname + "!" + client->username + "@localhost PART " + chan->name + " : Disconnected";
			chan->broadcast(partMsg, -1);
		}

		if (chan->members.empty())
		{
			delete chan;
			_channels.erase(it++);
		}
		else
		{
			++it;
		}
	}
}