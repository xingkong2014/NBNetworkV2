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

	using handler = std::function<HandleResult(char const *_clientIp, short _clientPort, NBRequest &_request,
		NBReadResponse &_readResponse, NBWriteResponse &_writeResponse)>;

	class NBServer
	{
#define DEFAULT_BUFFER_SIZE		1024
	public:
		static std::shared_ptr<NBServer> create(char const *_localip, short _port,
			handler const &_handlerCallback,
			unsigned _inBufferSize = DEFAULT_BUFFER_SIZE, unsigned _outBufferSize = DEFAULT_BUFFER_SIZE);
		virtual ~NBServer() = default;

		virtual void start() = 0;
		virtual void stop() = 0;
	};
}


