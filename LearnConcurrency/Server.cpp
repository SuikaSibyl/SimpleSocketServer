#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "Server.h"

Network::Server::Server() :clientList(MAX_CLIENT), sListen(NULL)
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
		int id = clientList.PushConn(sServer, std::string(inet_ntoa(saClient.sin_addr)), (int)saClient.sin_port);
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
		Packet::RecvState recvState;
		Packet::Header header;
		recvState = Packet::Packet::ReceiveByLength(sServer, (char*)&header, sizeof(header));
		if (recvState != Packet::RecvState::SUCCESS)
		{
			//client->connected = false;
			if (recvState == Packet::RecvState::UNEXPECTED) std::cout << "socket unexptected exit";
			if (recvState == Packet::RecvState::CLOSE) std::cout << "socket closed";

			// Clear the queue
			while (!server->emitList.EmitMap[id].empty()) server->emitList.EmitMap[id].pop();
			server->clientList.DelConn(id);

			break;
		}
		std::cout << "Header Type: " << header.packetType << " | Header Length: " << header.length << std::endl;

		char* con = new char[header.length];
		recvState = Packet::Packet::ReceiveByLength(sServer, con, header.length);
		if (recvState != Packet::RecvState::SUCCESS)
		{
			//client->connected = false;
			if (recvState == Packet::RecvState::UNEXPECTED) std::cout << "socket unexptected exit";
			if (recvState == Packet::RecvState::CLOSE) std::cout << "socket closed";

			// Clear the queue
			while (!server->emitList.EmitMap[id].empty()) server->emitList.EmitMap[id].pop();
			server->clientList.DelConn(id);

			break;
		}
		std::string content = con;
		std::cout << "receive content: " << content << std::endl;

		if (header.packetType == Packet::PacketType::REQ4TIME)
		{
			time_t t;
			time(&t); //获取time_t类型的当前时间
			std::string sendline = ctime(&t);
			std::cout << "send content: " << sendline << std::endl;

			Packet::Header header;
			header.packetType = Packet::PacketType::RES4TIME;
			header.length = sendline.length();
			server->emitList.PushBack(id, header, sendline);
		}
		else if (header.packetType == Packet::PacketType::REQ4NAME)
		{
			char server_name[128];
			std::string host_name;
			gethostname(server_name, 128); //返回本地主机的标准主机名
			host_name = server_name;
			std::cout << "send content to" << id << ": " << host_name << std::endl;

			Packet::Header header;
			header.packetType = Packet::PacketType::RES4NAME;
			header.length = host_name.length();
			server->emitList.PushBack(id, header, host_name);
		}
		else if (header.packetType == Packet::PacketType::REQ4LIST)
		{
			Packet::Header header;
			header.packetType = Packet::PacketType::RES4LIST;
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
		
		if (header.packetType == Packet::PacketType::DISCONNECT)
		{
			server->clientList.DelConn(id);

			// Clear the queue
			while (!server->emitList.EmitMap[id].empty()) server->emitList.EmitMap[id].pop();
			server->clientList.DelConn(id);

			std::cout << "Client " + std::to_string(id) + " disconnect!";

			break;
		}
	}
}


void Network::OutputLoop::operator()(Server* server)
{
	while (true)
	{
		for (auto iter = server->emitList.EmitMap.begin(); iter != server->emitList.EmitMap.end();)
		{
			int target = iter->first;
			bool occurDisconnect = false;
			std::shared_ptr<SOCKET> socekt = server->clientList.clientList[target].socket;
			while (iter->second.size() != 0)
			{
				EmitType emitType = iter->second.front();

				int ret = send(*socekt, (char*)&emitType.header, sizeof(emitType.header), 0);
				if (ret == SOCKET_ERROR)
				{
					std::cout << "send() failed! As client id:";
					std::cout << target;
					std::cout << " disconnected unexceptedly!\n";
					// Clear the queue
					while (!iter->second.empty()) iter->second.pop();
					iter = server->emitList.EmitMap.erase(iter);
					occurDisconnect = true;
					server->clientList.DelConn(target);
					if (iter == server->emitList.EmitMap.end())
						break;
					continue;
				}
				else
				{
					std::cout << "send out to " << target << std::endl;
				}

				ret = send(*socekt, emitType.contain.data(), emitType.header.length, 0);
				if (ret == SOCKET_ERROR)
				{
					std::cout << "send() failed! As client id:";
					std::cout << target;
					std::cout << " disconnected unexceptedly!\n";
					// Clear the queue
					while (!iter->second.empty()) iter->second.pop();
					iter = server->emitList.EmitMap.erase(iter);
					occurDisconnect = true;
					server->clientList.DelConn(target);

					if (iter == server->emitList.EmitMap.end())
						break;
					continue;
				}
				else
				{
					std::cout << "send out to " << target << std::endl;
				}
				if (occurDisconnect == false)
					iter->second.pop();
			}
			if (occurDisconnect == false)
				iter++;
		}
	}
}