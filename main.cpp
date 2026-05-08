#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>

bool g_running = true;

void signalHandler(int sig)
{
	(void)sig;
	std::cout << "\nSignal recieved, server stopping" << std::endl;
	g_running = false;
}

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" <<std::endl;
		return 1;
	}

	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);

	int port = std::atoi(av[1]);
	std::string password = av[2];
	Server ircServer(port, password);
	ircServer.init();
	ircServer.run();

	return 0;
}