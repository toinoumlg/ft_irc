#pragma once
#include "Client.hpp"

class Channel {
   public:
	Channel(const string& name, int creatorfd);
	~Channel();

   private:
	vector<int> _clients;
	string _name;
};