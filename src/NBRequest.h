#pragma once

namespace nbnetwork {
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
}