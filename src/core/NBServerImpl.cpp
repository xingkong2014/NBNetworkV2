#include "NBServerImpl.h"
#include "NBSession.h"

using namespace boost::asio;
namespace nbnetwork {
	std::shared_ptr<NBServer> NBServer::create(char const *_localip, short _port,
		newHandler const &_newCallback, ioHandler const &_handlerCallback, closeHandler const &_closeCallback,
		unsigned _inBufferSize, unsigned _outBufferSize)
	{
		return std::make_shared<NBServerImpl>(_localip, _port, _newCallback, _handlerCallback, _closeCallback, _inBufferSize, _outBufferSize);
	}

	NBServerImpl::NBServerImpl(char const *_localip, short port,
		newHandler const &_newCallback, ioHandler const &_handlerCallback, closeHandler const &_closeCallback,
		unsigned _inBufferSize, unsigned _outBufferSize)
		:m_acceptor(m_ioService, ip::tcp::endpoint(ip::address_v4::from_string(_localip), port))
		, m_newCallback(_newCallback)
		, m_handlerCallback(_handlerCallback)
		, m_closeCallback(_closeCallback)
		, m_inBufferSize(_inBufferSize)
		, m_outBufferSize(_outBufferSize)
	{
	}

	NBServerImpl::~NBServerImpl()
	{
		stop();
	}

	void NBServerImpl::start()
	{
		doAccept();

		for (unsigned i = 0; i < std::thread::hardware_concurrency(); i++)
		{
			m_workThreads.emplace_back([this] { this->m_ioService.run(); });
		}

		m_checkSessionThread = std::thread([&] {
			time_t lastCheckTime = time(nullptr);
			while (!m_stop)
			{
				time_t nowtime = time(nullptr);
				if (nowtime > lastCheckTime + 1 * 60)
				{
					std::lock_guard<std::mutex> lock(m_mutex);
					for (auto iter = m_sessions.cbegin(); iter != m_sessions.cend();++iter)
					{
						if (nowtime > (*iter)->ioTime() + 2 * 60)
						{
							(*iter)->stop();
						}
					}

					lastCheckTime = nowtime;
				}
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		});
	}

	void NBServerImpl::stop()
	{
		m_stop = true;
		if (m_checkSessionThread.joinable())
		{
			m_checkSessionThread.join();
		}

		m_ioService.stop();
		for (auto &thread : m_workThreads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}
	}

	void NBServerImpl::removeSession(std::shared_ptr<NBSession> const &_session)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		auto iter = m_sessions.find(_session);
		if (iter != m_sessions.end())
		{
			m_sessions.erase(iter);
		}
	}
	
	void NBServerImpl::doAccept()
	{
		auto session = std::make_shared<NBSession>(m_ioService, shared_from_this(), m_newCallback, m_handlerCallback, m_closeCallback, m_inBufferSize, m_outBufferSize);
		m_acceptor.async_accept(session->socket(), [this, session](boost::system::error_code ec) {
			if (!ec)
			{
				if (session->start())
				{
					std::lock_guard<std::mutex> lock(m_mutex);
					m_sessions.insert(session);
				}
			}

			doAccept();
		});
	}
}