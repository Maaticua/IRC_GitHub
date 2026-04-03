#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <sys/socket.h>
#include "Client.hpp"

class Channel
{
	public:
		std::string name;
		std::vector<Client *> members;
		std::vector<Client*> operators;

		Channel(std::string n) : name(n) {}

		void broadcast(std::string msg, int sender_fd)
		{
			for (size_t i = 0; i < members.size(); i++)
				if (members[i]->fd != sender_fd)
				{
					std::string final_msg = msg + "\r\n";
					send(members[i]->fd, final_msg.c_str(), final_msg.length(), 0);
				}
		}

};

#endif
