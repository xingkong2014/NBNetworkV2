#pragma once

#include <functional>
#include <memory>
#include "NBRequest.h"

namespace nbnetwork {
	enum class HandleResult
	{
		READ,
		WRITE,
		FAILED
	};

	using newHandler = std::function<bool(char const *_clientIp, unsigned short _clientPort)>;

	using ioHandler = std::function<HandleResult(char const *_clientIp, unsigned short _clientPort, NBRequest &_request,
		NBReadResponse &_readResponse, NBWriteResponse &_writeResponse)>;

	using closeHandler = std::function<void(char const *_clientIp, unsigned short _clientPort)>;

	class NBServer
	{
#define DEFAULT_BUFFER_SIZE		1024
	public:
		static std::shared_ptr<NBServer> create(char const *_localip, short _port,
			newHandler const &_newCallback, ioHandler const &_handlerCallback, closeHandler const &_closeCallback,
			unsigned _inBufferSize = DEFAULT_BUFFER_SIZE, unsigned _outBufferSize = DEFAULT_BUFFER_SIZE);
		virtual ~NBServer() = default;

		virtual void start() = 0;
		virtual void stop() = 0;
	};
}


