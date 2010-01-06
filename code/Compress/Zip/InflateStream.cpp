#include <cstring>
#include <zlib.h>
#include "Compress/Zip/InflateStream.h"

namespace traktor
{
	namespace compress
	{

class InflateImpl : public RefCountImpl< IRefCount >
{
public:
	InflateImpl(IStream* stream, uint32_t internalBufferSize)
	:	m_stream(stream)
	,	m_buf(internalBufferSize)
	,	m_startPosition(stream->tell())
	,	m_position(m_startPosition)
	{
		std::memset(&m_zstream, 0, sizeof(m_zstream));
		int rc = inflateInit(&m_zstream);
		T_ASSERT (rc == Z_OK);
	}

	void close()
	{
		inflateEnd(&m_zstream);
		m_stream = 0;
	}

	int read(void* block, int nbytes)
	{
		int rc;

		m_zstream.next_out = (Bytef*)block;
		m_zstream.avail_out = nbytes;

		for (;;)
		{
			if (m_zstream.avail_in == 0)
			{
				int nread = m_stream->read(&m_buf[0], int(m_buf.size()));
				if (!nread)
					break;

				m_zstream.next_in = &m_buf[0];
				m_zstream.avail_in = nread;
			}

			rc = inflate(&m_zstream, Z_SYNC_FLUSH);
			if (rc != Z_OK || m_zstream.avail_out == 0)
				break;
		}

		int nread = nbytes - m_zstream.avail_out;

		m_position += nread;
		return nread;
	}

	int setLogicalPosition(int position)
	{
		// Seeking backwards, restart from beginning.
		if (position < m_position)
		{
			inflateReset(&m_zstream);
			m_stream->seek(IStream::SeekSet, m_startPosition);
			m_zstream.next_in = 0;
			m_zstream.avail_in = 0;
			m_position = m_startPosition;
		}

		// Read dummy blocks until we're at the desired position.
		uint8_t dummy[1024];
		while (m_position < position)
		{
			int nread = read(dummy, std::min< int >(sizeof_array(dummy), position - m_position));
			if (nread <=  0)
				return -1;
		}

		return m_position;
	}

	int getLogicalPosition() const
	{
		return m_position;
	}

private:
	Ref< IStream > m_stream;
	z_stream m_zstream;
	std::vector< uint8_t > m_buf;
	int m_startPosition;
	int m_position;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.compress.InflateStream", InflateStream, IStream)

InflateStream::InflateStream(IStream* stream, uint32_t internalBufferSize)
:	m_impl(new InflateImpl(stream, internalBufferSize))
{
}

InflateStream::~InflateStream()
{
	close();
}

void InflateStream::close()
{
	if (m_impl)
	{
		m_impl->close();
		m_impl = 0;
	}
}

bool InflateStream::canRead() const
{
	return true;
}

bool InflateStream::canWrite() const
{
	return false;
}

bool InflateStream::canSeek() const
{
	return true;
}

int InflateStream::tell() const
{
	return m_impl->getLogicalPosition();
}

int InflateStream::available() const
{
	T_ASSERT (0);
	return 0;
}

int InflateStream::seek(SeekOriginType origin, int offset)
{
	T_ASSERT_M (origin != SeekEnd, L"Only SeekEnd is allowed on InflateStream");
	if (origin == SeekCurrent)
		offset += m_impl->getLogicalPosition();
	return m_impl->setLogicalPosition(offset);
}

int InflateStream::read(void* block, int nbytes)
{
	return m_impl->read(block, nbytes);
}

int InflateStream::write(const void* block, int nbytes)
{
	T_ASSERT (0);
	return 0;
}

void InflateStream::flush()
{
}

	}
}
