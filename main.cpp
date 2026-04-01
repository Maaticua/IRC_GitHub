#include "Server.hpp"
#include <iostream>
#include <cstdlib>

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" <<std::endl;
		return 1;
	}

	try
	{
		int port = std::atoi(av[1]);
		std::string password = av[2];

		Server ircServer(port, password);
		ircServer.init();
		ircServer.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}