#include "Server.h"

Network::Server::Server():clientList(MAX_CLIENT), sListen(NULL)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int ret;
	struct sockaddr_in saServer;//地址信息

	// WinSock Initialize
	wVersionRequested = MAKEWORD(2, 2);//WinSock DLL Version
	ret = WSAStartup(wVersionRequested, &wsaData);
	if (ret != 0)
	{
		printf("WSAStartup() failed!\n");
		return;
	}

	//Make sure WinSock DLL support 2.2：
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		printf("Invalid Winsock version!\n");
		return;
	}

	// Create socket，use TCP protocol
	// -------------------------------
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == INVALID_SOCKET)
	{
		WSACleanup();
		printf("socket() failed!\n");
		return;
	}
	
	//构建本地地址信息：
	saServer.sin_family = AF_INET;//地址家族
	saServer.sin_port = htons(SERVER_PORT);//注意转化为网络字节序
	saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//使用INADDR_ANY指示任意地址
	
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

	//侦听连接请求：
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

	//阻塞等待接受客户端连接：
	int length = sizeof(saClient);

	while (true)
	{
		std::shared_ptr<SOCKET> sServer = std::make_shared<SOCKET>(accept(sListen, (struct sockaddr*)&saClient, &length));
		if (*sServer == INVALID_SOCKET)
		{
			printf("accept() failed! code:%d\n", WSAGetLastError());
			closesocket(sListen);//关闭套接字
			WSACleanup();
			continue;
		}
		printf("Client Connected!: %s:%d\n",
			inet_ntoa(saClient.sin_addr), ntohs(saClient.sin_port));
		int id = clientList.PushConn(sServer,std::string(inet_ntoa(saClient.sin_addr)), (int)saClient.sin_port);
		auto f1 = std::bind(ReceiveResult(), sServer, id, this);
		threadPool.submit<std::function<void()>>(f1);

		std::cout << clientList.clientList.size();
	}
}

Network::Server::~Server()
{
	closesocket(sListen);//关闭套接字
	//closesocket(sServer);
	WSACleanup();
}

void Network::ReceiveResult::operator()(std::shared_ptr<SOCKET> sServer, int id, Server* server)
{
	while (true)
	{
		Packet::Header header;
		Packet::Packet::ReceiveByLength(sServer, (char*)&header, sizeof(header));
		std::cout << header.packetType << header.length;

		if (header.packetType == Packet::PacketType::REQ4LIST)
		{
			Packet::Header header;
			header.packetType = Packet::PacketType::LISTREQUEST;
			std::string data = server->clientList.Encapsule();
			std::cout << data;
			header.length = data.length();
			server->emitList.PushBack(id, header, data);
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
					std::cout << "send out to " << target;
				}

				ret = send(*socekt, emitType.contain.data() ,emitType.header.length, 0);
				if (ret == SOCKET_ERROR)
				{
					printf("send() failed!\n");
					continue;
				}
				else
				{
					std::cout << "send out to " << target;
				}

				iter->second.pop();
			}
		}
	}
}