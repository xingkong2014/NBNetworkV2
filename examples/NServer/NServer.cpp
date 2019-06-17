// NServer.cpp : Defines the entry point for the console application.
//

#include "NBServer.h"
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <cstring>

using namespace nbnetwork;
using namespace std::placeholders;

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

class ClientInfo
{
	friend struct ClientHash;
	friend struct ClientEqual;
public:
	ClientInfo(std::string const&_ip, short _port)
		:m_strIp(_ip)
		,m_port(_port)
	{}
protected:
private:
	std::string m_strIp;
	short m_port = 0;
};

struct ClientHash 
{
	std::size_t operator()(ClientInfo const &_info) const {
		return std::hash<std::string>()(_info.m_strIp) ^ std::hash<short>()(_info.m_port);
	}
};

struct ClientEqual 
{
	bool operator()(ClientInfo const &_left, ClientInfo const &_right) const {
		return _left.m_port == _right.m_port && _left.m_strIp == _right.m_strIp;
	}
};

class ClientContext
{
public:
	HandleResult dealPackage(NBRequest &_request, NBReadResponse &_readResponse, NBWriteResponse &_writeResponse)
	{
		auto totalSize = _request.totalDataSize();
		if (totalSize < sizeof(MyHead))
		{
			// continue read head
			_readResponse.setBuf(_request.lastIoBuffer().buffer() + _request.lastIoBuffer().size());
			_readResponse.setSize(sizeof(MyHead) - totalSize);
			return HandleResult::READ;
		}
		else
		{
			MyHead *pHead = (MyHead *)_request.startBuffer();
			if (totalSize < sizeof(MyHead) + pHead->m_bodysize)
			{
				// continue read body
				_readResponse.setBuf(_request.lastIoBuffer().buffer() + _request.lastIoBuffer().size());
				_readResponse.setSize(sizeof(MyHead) + pHead->m_bodysize - totalSize);
				return HandleResult::READ;
			}
			switch (pHead->m_command)
			{
			case MyCommand::Ping:
				return dealPingPackage(_request, _readResponse, _writeResponse);
				break;
			default:
				break;
			}
		}

		return HandleResult::FAILED;
	}
protected:
private:
	HandleResult dealPingPackage(NBRequest &_request, NBReadResponse &_readResponse, NBWriteResponse &_writeResponse)
	{
		MyHead *pHead = (MyHead *)_request.startBuffer();
		MyBody *pBody = (MyBody *)(pHead + 1);

		std::cout << "data : " << pBody->m_value << std::endl;
		_writeResponse.setSize(sizeof(MyHead) + sizeof(MyBody));
		memcpy(_writeResponse.buffer(), pHead, sizeof(MyHead)); // copy head
		MyHead *pOutHead = (MyHead *)_writeResponse.buffer();
		pOutHead->m_status = 1;
		pOutHead->m_command = MyCommand::Pong;
		MyBody *pOutBody = (MyBody *)(pOutHead + 1);
		strcpy(pOutBody->m_value, "world");

		return HandleResult::WRITE;
	}
};

class ClientManager
{
public:
	HandleResult dealPackage(char const *_clientIp, unsigned short _clientPort, NBRequest &_request,
		NBReadResponse &_readResponse, NBWriteResponse &_writeResponse)
	{
		return m_clients[ClientInfo(_clientIp, _clientPort)].dealPackage(_request, _readResponse, _writeResponse);
	}

	void clientClose(char const *_clientIp, unsigned short _clientPort)
	{
		m_clients.erase(ClientInfo(_clientIp, _clientPort));
		std::cout << "client : " << _clientIp << ", port : " << _clientPort << " closed." << std::endl;
	}
protected:
private:
	std::unordered_map<ClientInfo, ClientContext, ClientHash, ClientEqual> m_clients;
};

int main()
{
	ClientManager clientManager;

	auto s = NBServer::create("127.0.0.1", 7195, std::bind(&ClientManager::dealPackage, &clientManager, _1, _2, _3, _4, _5),
		std::bind(&ClientManager::clientClose, &clientManager, _1, _2));

	s->start();

	getchar();

	s->stop();

    return 0;
}

