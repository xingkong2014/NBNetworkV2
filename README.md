# NBNetworkV2
基于boost::asio的更通用的网络库
除了boost以外，还依赖cmake 1.34

## 编译：

    mkdir build
    cd build
    cmake ..
    make
会在build目录生成`libNetworkV2.a`文件

## 例子
### 服务端

    auto s = NBServer::create("127.0.0.1", 7195, dealPackage);
	s->start();

`dealPackage()`函数原型为

	HandleResult dealPackage(char const *_clientIp, short _clientPort, NBRequest &_request,
		NBReadResponse &_readResponse, NBWriteResponse &_writeResponse)
	{
	}

参数`_clientIp`和`_clientPort`表示连接的客户端的IP和端口，`_request`表示客户端发过来的数据包，`_readResponse`表示需要继续读的缓冲区，`_writeResponse`表示需要写的缓冲区。

	class RawBuffer
	{
	public:
		RawBuffer() = default;
		RawBuffer(char *_buf, std::size_t bufSize)
			:m_buffer(_buf)
			, m_bufSize(bufSize) {}

		char *buffer() { return m_buffer; }
		char const *buffer() const { return m_buffer; }
		std::size_t size() const { return m_bufSize; }

		void setSize(std::size_t _size) { m_bufSize = _size; }
		void setBuf(char *_buf) { m_buffer = _buf; }
	protected:
	private:
		char *m_buffer = nullptr;
		std::size_t m_bufSize = 0;
	};

	class NBRequest
	{
	public:
		NBRequest(char const *_pStart, RawBuffer const &_ioBuffer)
			:m_pStart(_pStart)
			,m_IoBuffer(_ioBuffer)
		{}

		char const *startBuffer() const { return m_pStart; }
		RawBuffer &lastIoBuffer() { return m_IoBuffer; }
		std::size_t totalDataSize() const { return m_IoBuffer.buffer() - m_pStart + m_IoBuffer.size(); }
	protected:
	private:
		char const *m_pStart = nullptr;
		RawBuffer m_IoBuffer;
	};

	using NBReadResponse = RawBuffer;
	using NBWriteResponse = RawBuffer;

返回值`HandleResult`表示需要继续读还是写。

	enum class HandleResult
	{
		READ,
		WRITE,
		FAILED
	};

例如当前`_request`中的数据还不够，还需要读，则在`_readResponse`中设置继续读取数据的位置和大小(大小设置为0表示读取任意长度，否则会一直读取到要求的长度才会返回)，并返回`HandleResult::READ`。
如果`_request`中的数据中的数据足够，并且处理完成后需要向客户端发送回应包，则在`_writeResponse`中保存好需要发送的数据，并返回`HandleResult::WRITE`。

### 客户端

    auto client = NBClient::create();
    client->connect("127.0.0.1", 7195, 5000);

`connect`前两个参数表示服务端ip和端口，第三个参数表示最多等待连接的超时时间(ms)。
连接成功后通过`sendData()`和`recvData()`来发送和接收数据。