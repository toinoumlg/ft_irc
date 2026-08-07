#pragma once
#include "Client.hpp"

class Channel {
   public:
	Channel(const std::string& name, int creatorfd);
	~Channel();

   private:
	std::vector<int> _clients;
	std::string _name;
};