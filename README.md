*This project has been created as part of the 42 curriculum by <macaruan> and <awaegaert>*

## Description
ft_irc is a custom Internet Relay Chat (IRC) server written in C++98.
This project aims to recreate the core functionalities of an IRC server, including client authentication, channel creation, private messaging, and operator privileges. It operates using non-blocking I/O and multiplexing via `poll()`.

## Instructions
To compile the project, run the following command at the root of the repository:

make

To start the server, execute:

./ircserv <port> <password>

Example: `./ircserv 6667 my_password`

## Resources
* IRC Protocol Specification (RFC 1459 / RFC 2812)
* Beej's Guide to Network Programming
* AI Tools (e.g., ChatGPT/Claude/Gemini) were used occasionally as a coding assistant to understand complex C++98 syntax, network logic debugging, and to help structure certain string parsing loops (like the MODE command flags).