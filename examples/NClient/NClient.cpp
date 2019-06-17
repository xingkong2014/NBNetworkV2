// NClient.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <vector>
#include "NBClient.h"

using namespace nbnetwork;
using namespace std;

#pragma pack(1)
enum MyCommand
{
	Ping = 1,
	Pong
};

struct MyHead
{
	std::uint32_t m_version;
	std::uint32_t m_command;
	std::uint32_t m_status;
	std::uint32_t m_bodysize;
};

struct MyBody
{
	char m_value[16]{};
};
#pragma pack()

int main()
{
	{
		auto client = NBClient::create();
		bool success = client->connect("127.0.0.1", 7195, 5000);
		if (success)
		{
			std::cout << "success" << std::endl;

			std::vector<char> buf(512);
			MyHead *pHead = (MyHead *)&buf[0];
			pHead->m_command = MyCommand::Ping;
			pHead->m_bodysize = sizeof(MyBody);
			MyBody *pBody = (MyBody *)(pHead + 1);
			strcpy(pBody->m_value, "hello");
			auto ec = client->sendData(&buf[0], sizeof(MyHead) + sizeof(MyBody));
			if (!ec)
			{
				unsigned recvSize = 0;
				ec = client->recvData(&buf[0], sizeof(MyHead));
				if (!ec)
				{
					if (pHead->m_command == MyCommand::Pong && pHead->m_bodysize == sizeof(MyBody))
					{
						ec = client->recvData(&buf[pHead->m_bodysize], pHead->m_bodysize);
						if (!ec)
						{
							std::cout << "recv : " << pBody->m_value << std::endl;
						}
					}
				}
				else {
					std::cout << "recvData failed : " << ec.message() << std::endl;
				}
			}
			else {
				std::cout << "sendData failed : " << ec.message() << std::endl;
			}
		}
		else {
			std::cout << "failed" << std::endl;
		}

		getchar();
	}
	getchar();
	return 0;
}

