#pragma once

namespace Packet
{
	enum RecvState
	{
		SUCCESS,
		UNEXPECTED,
		CLOSE
	};

	enum PacketType
	{
		NOTAPACKET,
		//��Ϣ����
		REQ4NAME,
		REQ4TIME,
		REQ4LIST,
		SENDINFO,
		DISCONNECT,
		//��Ӧ����
		RES4NAME,
		RES4TIME,
		RES4LIST,
		INFO
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
		static RecvState ReceiveByLength(std::shared_ptr<SOCKET> sServer, char* ptr, int length)
		{
			int nLeft = length;
			while (nLeft > 0)
			{
				//�������ݣ�
				int ret = recv(*sServer, ptr, nLeft, 0);
				if (ret == SOCKET_ERROR)
				{
					printf("recv() failed!\n");
					return RecvState::UNEXPECTED;
				}

				//�ͻ����Ѿ��ر�����
				if (ret == 0)
				{
					printf("client has closed the connection!\n");
					return RecvState::CLOSE;
				}
				nLeft -= ret;
				ptr += ret;
			}
			return RecvState::SUCCESS;
		}
	};
}

