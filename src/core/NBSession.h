#pragma once

#include <boost/asio.hpp>
#include <vector>
#include "../NBServer.h"

namespace nbnetwork {
	struct NBHead;
	class NBServerImpl;
	class NBSession : public std::enable_shared_from_this<NBSession>
	{
	public:
		NBSession(boost::asio::io_service &_ioService, std::shared_ptr<NBServerImpl> const &_pServer,
			ioHandler &_handlerCallback, closeHandler &_closeCallback,
			unsigned _inBufferSize, unsigned _outBufferSize);

		~NBSession();

		boost::asio::ip::tcp::socket &socket() { return m_socket; }
		void start();
		void stop();
		time_t ioTime() const { return m_lastIOTime; }

		NBSession(NBSession const&) = delete;
		NBSession &operator=(NBSession const &) = delete;
	private:
		void doRead(char *pBuffer, size_t size);
		void doWrite(char *pBuffer, size_t size);
		void readComplete(char *_pBuffer, boost::system::error_code _ec, std::size_t _bytes_transferred);

		boost::asio::ip::tcp::socket m_socket;

		std::string m_strClientIp;
		unsigned short m_port;

		std::atomic<time_t> m_lastIOTime{ 0 };

		std::vector<char> m_inBuffer;
		std::vector<char> m_outBuffer;

		RawBuffer m_writeDataInfo;

		ioHandler &m_handlerCallback;
		closeHandler &m_closeCallback;
		std::shared_ptr<NBServerImpl> m_pServer;
	};

}