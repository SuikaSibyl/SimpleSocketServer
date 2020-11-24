#pragma once

namespace Packet
{
	enum PacketType 
	{
		REQ4LIST,
		LISTREQUEST
	};

	struct Header
	{
		PacketType packetType;
		int length;
	};

	class Packet
	{

	};

	void ReceiveByLength(std::shared_ptr<SOCKET> sServer, char* ptr, int length)
	{
		int nLeft = length;
		while (nLeft > 0)
		{
			//�������ݣ�
			int ret = recv(*sServer, ptr, nLeft, 0);
			if (ret == SOCKET_ERROR)
			{
				printf("recv() failed!\n");
				break;
			}

			if (ret == 0) //�ͻ����Ѿ��ر�����
			{
				printf("client has closed the connection!\n");
				break;
			}
			nLeft -= ret;
			ptr += ret;
		}
	}
}

