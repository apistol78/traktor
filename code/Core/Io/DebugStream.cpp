#include <cstring>
#include "Core/Io/DebugStream.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace
	{

int32_t getRange(int32_t index)
{
	return 4 << index;
}

int32_t getIndex(int64_t size, int32_t maxIndex)
{
	for (int32_t i = 0; i < maxIndex; ++i)
	{
		if (size <= getRange(i))
			return i;
	}
	return maxIndex;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.DebugStream", DebugStream, IStream);

DebugStream::DebugStream(IStream* stream)
:	m_stream(stream)
,	m_canReadCalls(0)
,	m_canWriteCalls(0)
,	m_canSeekCalls(0)
,	m_tellCalls(0)
,	m_availableCalls(0)
,	m_seekCalls(0)
,	m_readCalls(0)
,	m_writeCalls(0)
,	m_flushCalls(0)
,	m_readTotal(0)
,	m_writeTotal(0)
,	m_readIoTime(0.0)
,	m_writeIoTime(0.0)
,	m_flushIoTime(0.0)
{
	std::memset(m_readTotals, 0, sizeof(m_readTotals));
	std::memset(m_writeTotals, 0, sizeof(m_writeTotals));
}

DebugStream::~DebugStream()
{
	T_ASSERT (!m_stream)
	dump(log::info);
}

void DebugStream::close()
{
	T_ASSERT (m_stream)
	m_stream->close();
	m_stream = 0;
}

bool DebugStream::canRead() const
{
	T_ASSERT (m_stream)
	m_canReadCalls++;
	return m_stream->canRead();
}

bool DebugStream::canWrite() const
{
	T_ASSERT (m_stream)
	m_canWriteCalls++;
	return m_stream->canWrite();
}

bool DebugStream::canSeek() const
{
	T_ASSERT (m_stream)
	m_canSeekCalls++;
	return m_stream->canSeek();
}

int64_t DebugStream::tell() const
{
	T_ASSERT (m_stream)
	m_tellCalls++;
	return m_stream->tell();
}

int64_t DebugStream::available() const
{
	T_ASSERT (m_stream)
	m_availableCalls++;
	return m_stream->available();
}

int64_t DebugStream::seek(SeekOriginType origin, int64_t offset)
{
	T_ASSERT (m_stream)
	m_seekCalls++;
	return m_stream->seek(origin, offset);
}

int64_t DebugStream::read(void* block, int64_t nbytes)
{
	T_ASSERT (m_stream)
	m_readCalls++;
	m_timer.start();
	int64_t nread = m_stream->read(block, nbytes);
	m_timer.stop();
	if (nread > 0)
	{
		int32_t index = getIndex(nread, sizeof_array(m_readTotals) - 1);
		m_readTotals[index]++;
		m_readTotal += nread;
	}
	m_readIoTime += m_timer.getElapsedTime();
	return nread;
}

int64_t DebugStream::write(const void* block, int64_t nbytes)
{
	T_ASSERT (m_stream)
	m_writeCalls++;
	m_timer.start();
	int64_t nwritten = m_stream->write(block, nbytes);
	m_timer.stop();
	if (nwritten > 0)
	{
		int64_t index = getIndex(nwritten, sizeof_array(m_writeTotals) - 1);
		m_writeTotals[index]++;
		m_writeTotal += nwritten;
	}
	m_writeIoTime += m_timer.getElapsedTime();
	return nwritten;
}

void DebugStream::flush()
{
	T_ASSERT (m_stream)
	m_flushCalls++;
	m_timer.start();
	m_stream->flush();
	m_timer.stop();
	m_flushIoTime += m_timer.getElapsedTime();
}

void DebugStream::dump(OutputStream& os) const
{
	os << L"-- Calls --" << Endl;
	os << m_canReadCalls << L" canRead call(s)" << Endl;
	os << m_canWriteCalls << L" canWrite call(s)" << Endl;
	os << m_canSeekCalls << L" canSeek call(s)" << Endl;
	os << m_tellCalls << L" tell call(s)" << Endl;
	os << m_availableCalls << L" available call(s)" << Endl;
	os << m_seekCalls << L" seek call(s)" << Endl;
	os << m_readCalls << L" read call(s)" << Endl;
	os << m_writeCalls << L" write call(s)" << Endl;
	os << m_flushCalls << L" flush call(s)" << Endl;
	os << Endl;

	os << L"-- Throughput --" << Endl;
	os << m_readTotal << L" byte(s) read (" << (m_readCalls > 0 ? m_readTotal / m_readCalls : 0) << L" byte(s) / call)" << Endl;
	os << m_writeTotal << L" byte(s) written (" << (m_writeCalls > 0 ? m_writeTotal / m_writeCalls : 0) << L" byte(s) / call)" << Endl;

	os << Endl;
	os << L"Read block sizes" << Endl;
	
	int32_t last = 0;
	for (int i = 0; i < sizeof_array(m_readTotals); ++i)
	{
		os << last << L" -> " << int32_t(getRange(i) - 1) << L" - " << m_readTotals[i] << Endl;
		last = getRange(i);
	}

	os << Endl;
	os << L"Write block sizes" << Endl;
	
	last = 0;
	for (int i = 0; i < sizeof_array(m_writeTotals); ++i)
	{
		os << last << L" -> " << int32_t(getRange(i) - 1) << L" - " << m_writeTotals[i] << Endl;
		last = getRange(i);
	}

	os << Endl;

	os << L"-- IO time --" << Endl;
	os << m_readIoTime << L" s read IO time" << Endl;
	os << m_writeIoTime << L" s write IO time" << Endl;
	os << m_flushIoTime << L" s flush IO time" << Endl;
}

}
