#pragma once

#include "ThreadPool.h"
#include "ClientList.h"
#include "EmitList.h"

#pragma comment(lib,"ws2_32.lib")  
#define SERVER_PORT	6666 //�����˿�
#define MAX_CLIENT 10

namespace Network
{
	class Server
	{
	public:
		Server();
		~Server();
		void enterMainLoop();

		friend class ReceiveResult;
		friend class OutputLoop;

	private:
		ThreadPool threadPool;
		ClientList clientList;
		SOCKET sListen; //�����׽��֣������׽���
		EmitList emitList;
	};

	class ReceiveResult
	{
	public:
		void operator()(std::shared_ptr<SOCKET> sServer, int id, Server* server);
	};

	class OutputLoop
	{
	public:
		void operator()(Server* server);
	};
}