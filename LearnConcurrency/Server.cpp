#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "Server.h"

Network::Server::Server() :clientList(MAX_CLIENT), sListen(NULL)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int ret;
	struct sockaddr_in saServer;//��ַ��Ϣ

	// WinSock Initialize
	wVersionRequested = MAKEWORD(2, 2);//WinSock DLL Version
	ret = WSAStartup(wVersionRequested, &wsaData);
	if (ret != 0)
	{
		printf("WSAStartup() failed!\n");
		return;
	}

	//Make sure WinSock DLL support 2.2��
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		printf("Invalid Winsock version!\n");
		return;
	}

	// Create socket��use TCP protocol
	// -------------------------------
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == INVALID_SOCKET)
	{
		WSACleanup();
		printf("socket() failed!\n");
		return;
	}

	//�������ص�ַ��Ϣ��
	saServer.sin_family = AF_INET;//��ַ����
	saServer.sin_port = htons(SERVER_PORT);//ע��ת��Ϊ�����ֽ���
	saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//ʹ��INADDR_ANYָʾ�����ַ

	// Bind:
	// -----
	ret = bind(sListen, (struct sockaddr*)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		printf("bind() failed! code:%d\n", WSAGetLastError());
		closesocket(sListen);
		WSACleanup();
		return;
	}

	//������������
	ret = listen(sListen, 5);
	if (ret == SOCKET_ERROR)
	{
		printf("listen() failed! code:%d\n", WSAGetLastError());
		closesocket(sListen);
		WSACleanup();
		return;
	}
}

void Network::Server::enterMainLoop()
{
	struct sockaddr_in saClient;

	std::cout << "Waiting for client connecting!" << std::endl;
	std::cout << "tips : Ctrl+c to quit!" << std::endl;

	auto f0 = std::bind(OutputLoop(), this);
	threadPool.submit<std::function<void()>>(f0);

	//�����ȴ����ܿͻ������ӣ�
	int length = sizeof(saClient);

	while (true)
	{
		std::shared_ptr<SOCKET> sServer = std::make_shared<SOCKET>(accept(sListen, (struct sockaddr*)&saClient, &length));
		if (*sServer == INVALID_SOCKET)
		{
			printf("accept() failed! code:%d\n", WSAGetLastError());
			closesocket(sListen);//�ر��׽���
			WSACleanup();
			continue;
		}
		printf("Client Connected!: %s:%d\n",
			inet_ntoa(saClient.sin_addr), ntohs(saClient.sin_port));
		int id = clientList.PushConn(sServer, std::string(inet_ntoa(saClient.sin_addr)), (int)saClient.sin_port);
		auto f1 = std::bind(ReceiveResult(), sServer, id, this);
		threadPool.submit<std::function<void()>>(f1);

		std::cout << clientList.clientList.size();
	}
}

Network::Server::~Server()
{
	closesocket(sListen);//�ر��׽���
	//closesocket(sServer);
	WSACleanup();
}

void Network::ReceiveResult::operator()(std::shared_ptr<SOCKET> sServer, int id, Server* server)
{
	while (true)
	{
		Packet::Header header;
		Packet::Packet::ReceiveByLength(sServer, (char*)&header, sizeof(header));
		std::cout << "Header Type: " << header.packetType << " | Header Length: " << header.length << std::endl;

		char* con = new char[header.length];
		Packet::Packet::ReceiveByLength(sServer, con, header.length);
		std::string content = con;
		std::cout << "receive content: " << content << std::endl;

		if (header.packetType == Packet::PacketType::REQ4TIME)
		{
			time_t t;
			time(&t); //��ȡtime_t���͵ĵ�ǰʱ��
			std::string sendline = ctime(&t);
			std::cout << "send content: " << sendline << std::endl;

			Packet::Header header;
			header.packetType = Packet::PacketType::INFO;
			header.length = sendline.length();
			server->emitList.PushBack(id, header, sendline);
		}
		else if (header.packetType == Packet::PacketType::REQ4NAME)
		{
			char server_name[128];
			std::string host_name;
			gethostname(server_name, 128); //���ر��������ı�׼������
			host_name = server_name;
			std::cout << "send content to" << id << ": " << host_name << std::endl;

			Packet::Header header;
			header.packetType = Packet::PacketType::INFO;
			header.length = host_name.length();
			server->emitList.PushBack(id, header, host_name);
		}
		else if (header.packetType == Packet::PacketType::REQ4LIST)
		{
			Packet::Header header;
			header.packetType = Packet::PacketType::INFO;
			std::string data = server->clientList.Encapsule();
			std::cout << "send content to" << id << ": " << data << std::endl;
			header.length = data.length();
			server->emitList.PushBack(id, header, data);
		}
		else if (header.packetType == Packet::PacketType::SENDINFO)
		{
			int id2 = atoi(content.substr(0, 3).c_str());
			std::string content2 = content.substr(3);
			std::cout << "send content to" << id2 << ": " << content2 << std::endl;
			Packet::Header header;
			header.packetType = Packet::PacketType::INFO;
			header.length = content2.length();
			server->emitList.PushBack(id2, header, content2);

			std::string ans = "Message send successfully!";
			std::cout << "send content to" << id << ": " << ans << std::endl;
			Packet::Header header2;
			header2.packetType = Packet::PacketType::INFO;
			header2.length = ans.length();
			server->emitList.PushBack(id, header2, ans);
		}
	}
}


void Network::OutputLoop::operator()(Server* server)
{
	while (true)
	{
		for (auto iter = server->emitList.EmitMap.begin(); iter != server->emitList.EmitMap.end(); iter++)
		{
			int target = iter->first;
			std::shared_ptr<SOCKET> socekt = server->clientList.clientList[target].socket;
			while (iter->second.size() != 0)
			{
				EmitType emitType = iter->second.front();

				int ret = send(*socekt, (char*)&emitType.header, sizeof(emitType.header), 0);
				if (ret == SOCKET_ERROR)
				{
					printf("send() failed!\n");
					continue;
				}
				else
				{
					std::cout << "send out to " << target << std::endl;
				}

				ret = send(*socekt, emitType.contain.data(), emitType.header.length, 0);
				if (ret == SOCKET_ERROR)
				{
					printf("send() failed!\n");
					continue;
				}
				else
				{
					std::cout << "send out to " << target << std::endl;
				}

				iter->second.pop();
			}
		}
	}
}