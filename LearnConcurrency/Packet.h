#pragma once

namespace Packet
{
	enum PacketType 
	{
		NOTAPACKET,
		REQ4LIST,
		LISTREQUEST
	};

	struct Header
	{
		PacketType packetType;
		int length;

		Header() { packetType = PacketType::NOTAPACKET; length = 0; }
	};

	class Packet
	{
	public:
		static void ReceiveByLength(std::shared_ptr<SOCKET> sServer, char* ptr, int length)
		{
			int nLeft = length;
			while (nLeft > 0)
			{
				//接收数据：
				int ret = recv(*sServer, ptr, nLeft, 0);
				if (ret == SOCKET_ERROR)
				{
					printf("recv() failed!\n");
					break;
				}

				if (ret == 0) //客户端已经关闭连接
				{
					printf("client has closed the connection!\n");
					break;
				}
				nLeft -= ret;
				ptr += ret;
			}
		}
	};
}

