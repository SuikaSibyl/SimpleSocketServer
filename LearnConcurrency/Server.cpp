#include "Server.h"

Network::Server::Server()
{

}

//WORD wVersionRequested;
//WSADATA wsaData;
//int ret, nLeft, length;
//SOCKET sListen, sServer; //�����׽��֣������׽���
//struct sockaddr_in saServer, saClient;//��ַ��Ϣ
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
////Make sure WinSock DLL support 2.2��
//if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
//{
//	WSACleanup();
//	printf("Invalid Winsock version!\n");
//	return;
//}
//
//// Create socket��use TCP protocol
//// -------------------------------
//sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//if (sListen == INVALID_SOCKET)
//{
//	WSACleanup();
//	printf("socket() failed!\n");
//	return;
//}
//
////�������ص�ַ��Ϣ��
//saServer.sin_family = AF_INET;//��ַ����
//saServer.sin_port = htons(SERVER_PORT);//ע��ת��Ϊ�����ֽ���
//saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//ʹ��INADDR_ANYָʾ�����ַ
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
////������������
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
////�����ȴ����ܿͻ������ӣ�
//length = sizeof(saClient);
//
//ThreadPool threadPool;
//while (true)
//{
//	std::shared_ptr<SOCKET> sServer = std::make_shared<SOCKET>(accept(sListen, (struct sockaddr*)&saClient, &length));
//	if (*sServer == INVALID_SOCKET)
//	{
//		printf("accept() failed! code:%d\n", WSAGetLastError());
//		closesocket(sListen);//�ر��׽���
//		WSACleanup();
//		continue;
//	}
//	printf("Accepted client: %s:%d\n",
//		inet_ntoa(saClient.sin_addr), ntohs(saClient.sin_port));
//	ReceiveResult receiveResult;
//	auto f1 = std::bind(ReceiveResult(), sServer);
//	threadPool.submit<std::function<void()>>(f1);
//
//	//����Ԥ��Э�飬�ͻ��˽�����һ��ѧ������Ϣ��
//
//}
//
//closesocket(sListen);//�ر��׽���
//closesocket(sServer);
//WSACleanup();