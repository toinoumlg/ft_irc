#include "Helper.hpp"
#include "Response.hpp"
#include "Server.hpp"

using namespace std;

// Connection commands
// [francais] Modification: valide sans réécrire le nom d'utilisateur, accepte
// un nom reel vide et refus d'une identité impossible à annoncer dans la
// bienvenue.
void Server::User(vector<string>& args, Client& client) {
	if (!client.HasSetPassword())
		return QueueMessage(Response::Build(ERR_NOTREGISTERED, client,
		                                    ":You have not registered"));

	if (args.size() < 4 || args[0].empty() || args[1].empty() ||
	    args[2].empty())
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "USER :Need more parameters"));
	if (client.IsRegistered())
		return QueueMessage(
		    Response::Build(ERR_ALREADYREGISTRED, client,
			                ":Unauthorized command (already registered)"));

	if (!IsValidUsername(args[0]))
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "USER :Invalid username"));
	// [francais] Réserve neuf octets pour le pseudo destinataire de la réponse
	// numérique
	// et neuf pour le pseudo dans l'identité complète, même si NICK arrive
	// après USER.
	const size_t welcome_size = Response::LOCALHOST.size() + args[0].size() +
	                            client.GetHost().size() + 28;
	if (welcome_size > 510) {
		QueueMessage(Response::Build(
		    "ERROR", ":Identity cannot fit in an IRC welcome message"));
		return client.MarkClosing("Identity too long");
	}
	client.SetUser(args);
	if (!client.IsRegistered() && client.SetRegistered())
		NotifyWelcome();
}
