#include <boost/bind.hpp>
#include <iostream>
#include "NBSession.h"
#include "NBServerImpl.h"

using namespace boost::asio;
namespace nbnetwork {
	NBSession::NBSession(boost::asio::io_service &_ioService, std::shared_ptr<NBServerImpl> const &_pServer,
		ioHandler &_handlerCallback, closeHandler &_closeCallback,
		unsigned _inBufferSize, unsigned _outBufferSize)
		:m_socket(_ioService)
		, m_inBuffer(_inBufferSize)
		, m_outBuffer(_outBufferSize)
		,m_handlerCallback(_handlerCallback)
		,m_closeCallback(_closeCallback)
		,m_pServer(_pServer)
	{
	}

	NBSession::~NBSession()
	{
		std::cout << "NBSession close" << std::endl;
		m_closeCallback(m_strClientIp.c_str(), m_port);
	}

	void NBSession::start()
	{
		m_strClientIp = m_socket.remote_endpoint().address().to_string();
		m_port = m_socket.remote_endpoint().port();
		doRead(&m_inBuffer[0], 0);
	}

	void NBSession::stop()
	{
		m_socket.close();
	}
	
	void NBSession::doRead(char *pBuffer, std::size_t size)
	{
		if (size == 0 || pBuffer + size > &m_inBuffer[0] + m_inBuffer.size())
		{
			auto newSize = m_inBuffer.size() - (pBuffer - &m_inBuffer[0]); // rest size
			m_socket.async_read_some(buffer(pBuffer, newSize), boost::bind(&NBSession::readComplete, this, pBuffer,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
		else {
			async_read(m_socket, buffer(pBuffer, size), transfer_all(), boost::bind(&NBSession::readComplete, this, pBuffer,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
	}

	void NBSession::doWrite(char *pBuffer, std::size_t size)
	{
		auto self(shared_from_this());
		async_write(m_socket, buffer(pBuffer, size), transfer_all(), [this, self](boost::system::error_code _ec, std::size_t) {
			if (_ec)
			{
				// error
				m_pServer->removeSession(shared_from_this());
			}
			else {
				doRead(&m_inBuffer[0], m_inBuffer.size());
			}
		});
	}

	void NBSession::readComplete(char *_pBuffer, boost::system::error_code _ec, std::size_t _bytes_transferred)
	{
		m_lastIOTime = time(nullptr);
		if (!_ec)
		{
			RawBuffer inBuf(_pBuffer, _bytes_transferred);
			NBRequest request(&m_inBuffer[0], inBuf);

			NBReadResponse readResponse;
			NBWriteResponse writeResponse(&m_outBuffer[0], m_outBuffer.size());

			auto ret = m_handlerCallback(m_strClientIp.c_str(), m_port, request, readResponse, writeResponse);
			if (ret == HandleResult::FAILED)
			{
				// error
				m_pServer->removeSession(shared_from_this());
			}
			else if (ret == HandleResult::READ)
			{
				doRead(readResponse.buffer(), readResponse.size());
			}
			else {
				doWrite(writeResponse.buffer(), writeResponse.size());
			}
		}
		else {
			// error
			m_pServer->removeSession(shared_from_this());
		}
	}
}
