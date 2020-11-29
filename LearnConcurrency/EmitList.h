#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include "Packet.h"

struct EmitType
{
	Packet::Header header;
	std::string contain;

	EmitType(Packet::Header iheader, std::string icontain)
	{
		header = iheader;
		contain = icontain;
	}
};

class EmitList
{
public:
	std::map<int, std::queue<EmitType>> EmitMap;
	void PushBack(int id, Packet::Header header, std::string contain)
	{
		EmitMap[id].push(EmitType(header, contain));
	}

private:

};