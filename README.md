*This project has been created as part of the 42 curriculum by <macaruan> and <awaegaert>*

## Description
ft_irc is a custom Internet Relay Chat (IRC) server written in C++98.
This project aims to recreate the core functionalities of an IRC server, including client authentication, channel creation, private messaging, and operator privileges. It operates using non-blocking I/O and multiplexing via `poll()`.

## Instructions
To compile the project, run the following command at the root of the repository:
\`\`\`bash
make
\`\`\`

To start the server, execute:
\`\`\`bash
./ircserv <port> <password>
\`\`\`
Example: `./ircserv 6667 my_password`

## Resources
* IRC Protocol Specification (RFC 1459 / RFC 2812)
* Beej's Guide to Network Programming
* AI Tools (e.g., ChatGPT/Claude/Gemini) were used occasionally as a coding assistant to understand complex C++98 syntax, network logic debugging, and to help structure certain string parsing loops (like the MODE command flags).

## How To Use

### 1. Connect with Netcat (Raw Protocol)
Netcat allows you to communicate with the server using raw IRC commands. The `-C` flag is mandatory as it enforces the `\r\n` (CRLF) line endings required by the IRC protocol.

Open a terminal and run:
\`\`\`bash
nc -C 127.0.0.1 6667
\`\`\`
To register and enter the server, you must type the following commands in this exact order:
1. `PASS my_password`
2. `NICK my_nickname`
3. `USER my_username 0 * :My Real Name`

### 2. Connect with Irssi (IRC Client)
Irssi is a terminal-based IRC client that handles the raw protocol formatting for you.
Open a terminal and run `irssi`. Once inside, type the following command to connect to your local server:
\`\`\`text
/connect 127.0.0.1 6667 my_password
\`\`\`
*(Irssi will automatically send the PASS, NICK, and USER commands in the background).*

---

### Available Commands
Below is the list of features implemented in this server.
*(Note: If you use Irssi, you usually type `/command`. If you use Netcat, you type the raw syntax).*

#### Basic Commands
* **PASS** : Authenticate with the server.
  * *Raw:* `PASS <password>`
* **NICK** : Set or change your nickname.
  * *Raw:* `NICK <nickname>`
  * *Irssi:* `/nick <nickname>`
* **USER** : Set your username and real name (used during initial registration).
  * *Raw:* `USER <username> 0 * :<realname>`
* **JOIN** : Join a channel (creates it if it doesn't exist).
  * *Raw:* `JOIN #<channel_name> [<password>]`
  * *Irssi:* `/join #<channel_name> [<password>]`
* **PART** : Leave a channel.
  * *Raw:* `PART #<channel_name> [:<reason>]`
  * *Irssi:* `/part #<channel_name> [reason]`
* **PRIVMSG** : Send a private message to a user or a channel.
  * *Raw:* `PRIVMSG <target> :<message>`
  * *Irssi:* `/msg <target> <message>`
* **QUIT** : Disconnect from the server.
  * *Raw:* `QUIT [:<reason>]`
  * *Irssi:* `/quit [reason]`
* **PING** : Check if the server is alive. The server will reply with `PONG`.
  * *Raw:* `PING :<server>`

#### Channel Operator Commands
The first user to join a newly created channel automatically becomes its Operator. Operators have exclusive rights to manage the channel.

* **KICK** : Eject a user from the channel.
  * *Raw:* `KICK #<channel_name> <nickname> [:<reason>]`
  * *Irssi:* `/kick <nickname> [reason]`
* **INVITE** : Invite a user to a channel (useful if the channel is invite-only).
  * *Raw:* `INVITE <nickname> #<channel_name>`
  * *Irssi:* `/invite <nickname> #<channel_name>`
* **TOPIC** : View or change the channel's topic.
  * *Raw:* `TOPIC #<channel_name> [:<new_topic>]`
  * *Irssi:* `/topic <new_topic>`
* **MODE** : Modify channel settings.
  * *Raw / Irssi:* `MODE #<channel_name> <+/- flags> [<arguments>]`
  * **Supported Flags:**
    * `+i` / `-i` : Set/remove Invite-only mode.
    * `+t` / `-t` : Set/remove restriction (only Operators can change the TOPIC).
    * `+k` / `-k` : Set/remove a channel password (key). *(Requires password as argument)*
    * `+l` / `-l` : Set/remove the maximum user limit. *(Requires number as argument)*
    * `+o` / `-o` : Give/take Operator privileges to/from a user. *(Requires nickname as argument)*