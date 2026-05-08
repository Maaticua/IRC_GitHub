if (!isMember)
		{
			chan->members.push_back(client);
			std::string joinMsg = ":" + client->nickname + "!" + client->username + "@localhost JOIN :" + chanName;
			chan->broadcast(joinMsg, -1); // Le broadcast actuel prévient tout le monde (y compris le client)

			// --- NOUVEAU : Les 3 informations obligatoires du protocole IRC ---

			// 1. Envoyer le sujet du salon (RPL_TOPIC ou RPL_NOTOPIC)
			if (chan->topic.empty())
				sendResponse(fd, "331 " + client->nickname + " " + chanName + " :No topic is set");
			else
				sendResponse(fd, "332 " + client->nickname + " " + chanName + " :" + chan->topic);

			// 2. Envoyer la liste des membres (RPL_NAMREPLY)
			std::string namesList = "353 " + client->nickname + " = " + chanName + " :";
			for (size_t i = 0; i < chan->members.size(); i++)
			{
				// Vérifier si le membre est opérateur pour ajouter le préfixe '@'
				bool isOp = false;
				for (size_t j = 0; j < chan->operators.size(); j++)
				{
					if (chan->operators[j]->fd == chan->members[i]->fd)
					{
						isOp = true;
						break;
					}
				}
				if (isOp)
					namesList += "@";
				namesList += chan->members[i]->nickname + " ";
			}
			sendResponse(fd, namesList);

			// 3. Envoyer la fin de la liste (RPL_ENDOFNAMES)
			sendResponse(fd, "366 " + client->nickname + " " + chanName + " :End of /NAMES list");
		}