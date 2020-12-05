#pragma once

#include <vector>
#include <string>
#include <map>

struct Connection
{
	int id;
	std::string address;
	int port;
	std::shared_ptr<SOCKET> socket;

	Connection() { id = -1; port = 0; }
	Connection(int iid, std::string iadd, int iport, std::shared_ptr<SOCKET> iSocket):
		id(iid),address(iadd),port(iport), socket(iSocket){}
};

class ClientList
{
public:
	std::map<int, Connection> clientList;
	std::vector<int> valid_ID;

public:
	ClientList(int maxSize)
	{
		for (int i = maxSize; i > 0; i--)
		{
			valid_ID.push_back(i);
		}
	}

	std::string Encapsule()
	{
		std::string ret;
		for (auto iter = clientList.begin(); iter != clientList.end(); iter++)
		{
			if (iter->second.id != -1)
			{
				ret += std::to_string(iter->second.id) + "/";
				ret += iter->second.address + "/";
				ret += std::to_string(iter->second.port) + "#";
			}
		}
		return ret;
	}

	int PushConn(std::shared_ptr<SOCKET> socket, std::string add, int port)
	{
		if (valid_ID.size() == 0)
			return false;
		int id = valid_ID.back();
		valid_ID.pop_back();
		clientList[id] = (Connection(id, add, port, socket));
		return id;
	}

	bool DelConn(int id)
	{
		valid_ID.push_back(id);
		for (auto iter = clientList.begin(); iter != clientList.end();)
		{
			if (iter->second.id == id)
			{
				iter = clientList.erase(iter);
				std::cout << "Disconnect happened. Client num now: " << clientList.size() << std::endl;
			}
			else
			{
				iter++;
			}
		}
		return true;
	}
};

