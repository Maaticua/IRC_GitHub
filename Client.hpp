#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
	public:
		int fd;
		std::string nickname;
		std::string username;
		bool has_pass;
		bool is_registered;

		Client(int f) : fd(f), has_pass(false), is_registered(false) {}
};

#endif