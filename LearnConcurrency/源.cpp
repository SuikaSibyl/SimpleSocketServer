#include <iostream>
#include <thread>
#include <stdio.h>
#include <winsock2.h>
#include "ThreadPool.h"

#pragma comment(lib,"ws2_32.lib")  
#define SERVER_PORT	6666 //�����˿�

//�ͻ�������������͵Ľṹ��
struct student
{
	char name[32];
	int age;
};

class ReceiveResult
{
public:
	void operator()(std::shared_ptr<SOCKET> sServer) const
	{
		student stu;
		int ret, nLeft, length;
		char* ptr;
		nLeft = sizeof(stu);
		ptr = (char*)&stu;
		while (nLeft > 0)
		{
			//�������ݣ�
			ret = recv(*sServer, ptr, nLeft, 0);
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

		if (!nLeft) //�Ѿ����յ�����������
			printf("name: %s\nage:%d\n", stu.name, stu.age);
	}
};

void main()
{
}