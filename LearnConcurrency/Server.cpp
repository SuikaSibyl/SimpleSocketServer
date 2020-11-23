#include "Server.h"

Network::Server::Server()
{

}

//WORD wVersionRequested;
//WSADATA wsaData;
//int ret, nLeft, length;
//SOCKET sListen, sServer; //侦听套接字，连接套接字
//struct sockaddr_in saServer, saClient;//地址信息
//struct student stu;
//char* ptr;
//
//// WinSock Initialize
//// ------------------
//wVersionRequested = MAKEWORD(2, 2);//WinSock DLL Version
//ret = WSAStartup(wVersionRequested, &wsaData);
//if (ret != 0)
//{
//	printf("WSAStartup() failed!\n");
//	return;
//}
////Make sure WinSock DLL support 2.2：
//if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
//{
//	WSACleanup();
//	printf("Invalid Winsock version!\n");
//	return;
//}
//
//// Create socket，use TCP protocol
//// -------------------------------
//sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//if (sListen == INVALID_SOCKET)
//{
//	WSACleanup();
//	printf("socket() failed!\n");
//	return;
//}
//
////构建本地地址信息：
//saServer.sin_family = AF_INET;//地址家族
//saServer.sin_port = htons(SERVER_PORT);//注意转化为网络字节序
//saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//使用INADDR_ANY指示任意地址
//
//// Bind:
//// -----
//ret = bind(sListen, (struct sockaddr*)&saServer, sizeof(saServer));
//if (ret == SOCKET_ERROR)
//{
//	printf("bind() failed! code:%d\n", WSAGetLastError());
//	closesocket(sListen);
//	WSACleanup();
//	return;
//}
//
////侦听连接请求：
//ret = listen(sListen, 5);
//if (ret == SOCKET_ERROR)
//{
//	printf("listen() failed! code:%d\n", WSAGetLastError());
//	closesocket(sListen);
//	WSACleanup();
//	return;
//}
//
//printf("Waiting for client connecting!\n");
//printf("tips : Ctrl+c to quit!\n");
////阻塞等待接受客户端连接：
//length = sizeof(saClient);
//
//ThreadPool threadPool;
//while (true)
//{
//	std::shared_ptr<SOCKET> sServer = std::make_shared<SOCKET>(accept(sListen, (struct sockaddr*)&saClient, &length));
//	if (*sServer == INVALID_SOCKET)
//	{
//		printf("accept() failed! code:%d\n", WSAGetLastError());
//		closesocket(sListen);//关闭套接字
//		WSACleanup();
//		continue;
//	}
//	printf("Accepted client: %s:%d\n",
//		inet_ntoa(saClient.sin_addr), ntohs(saClient.sin_port));
//	ReceiveResult receiveResult;
//	auto f1 = std::bind(ReceiveResult(), sServer);
//	threadPool.submit<std::function<void()>>(f1);
//
//	//按照预定协议，客户端将发来一个学生的信息：
//
//}
//
//closesocket(sListen);//关闭套接字
//closesocket(sServer);
//WSACleanup();