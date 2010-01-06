#include <cstring>
#include <zlib.h>
#include "Compress/Zip/DeflateStream.h"

namespace traktor
{
	namespace compress
	{

class DeflateImpl : public RefCountImpl< IRefCount >
{
public:
	DeflateImpl(IStream* stream, uint32_t internalBufferSize)
	:	m_stream(stream)
	,	m_buf(internalBufferSize)
	,	m_position(stream->tell())
	{
		std::memset(&m_zstream, 0, sizeof(m_zstream));

		int rc = deflateInit(&m_zstream, Z_DEFAULT_COMPRESSION);
		T_ASSERT (rc == Z_OK);

		m_zstream.next_out = (Bytef*)&m_buf[0];
		m_zstream.avail_out = (uInt)m_buf.size();
	}

	void close()
	{
		flush();
		deflateEnd(&m_zstream);
		m_stream = 0;
	}

	int write(const void* block, int nbytes)
	{
		int rc;

		m_zstream.next_in = (Bytef*)block;
		m_zstream.avail_in = nbytes;

		for (;;)
		{
			T_ASSERT (m_zstream.avail_out > 0);
			rc = deflate(&m_zstream, Z_SYNC_FLUSH);
			if (rc != Z_OK)
				return -1;

			if (m_zstream.avail_out == 0)
			{
				int nwritten = m_stream->write(&m_buf[0], int(m_buf.size()));
				if (nwritten != m_buf.size())
					return -1;

				m_zstream.next_out = (Bytef*)&m_buf[0];
				m_zstream.avail_out = (uInt)m_buf.size();
			}

			if (m_zstream.avail_in == 0)
				break;
		}

		m_position += nbytes;
		return nbytes;
	}

	void flush()
	{
		for (;;)
		{
			T_ASSERT (m_zstream.avail_in == 0);
			T_ASSERT (m_zstream.avail_out > 0);
			int rc = deflate(&m_zstream, Z_FULL_FLUSH);
			T_ASSERT (rc == Z_OK || rc == Z_BUF_ERROR);

			int nwrite = int(m_buf.size()) - m_zstream.avail_out;
			if (nwrite > 0)
			{
				T_ASSERT (nwrite <= int(m_buf.size()));
				int nwritten = m_stream->write(&m_buf[0], nwrite);
				T_ASSERT (nwritten == nwrite);
			}

			if (m_zstream.avail_out > 0)
				break;

			m_zstream.next_out = (Bytef*)&m_buf[0];
			m_zstream.avail_out = (uInt)m_buf.size();
		}
	}

	int getLogicalPosition() const
	{
		return m_position;
	}

private:
	Ref< IStream > m_stream;
	z_stream m_zstream;
	std::vector< uint8_t > m_buf;
	int m_position;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.compress.DeflateStream", DeflateStream, IStream)

DeflateStream::DeflateStream(IStream* stream, uint32_t internalBufferSize)
:	m_impl(new DeflateImpl(stream, internalBufferSize))
{
}

DeflateStream::~DeflateStream()
{
	close();
}

void DeflateStream::close()
{
	if (m_impl)
	{
		m_impl->close();
		m_impl = 0;
	}
}

bool DeflateStream::canRead() const
{
	return false;
}

bool DeflateStream::canWrite() const
{
	return true;
}

bool DeflateStream::canSeek() const
{
	return false;
}

int DeflateStream::tell() const
{
	T_ASSERT (m_impl);
	return m_impl->getLogicalPosition();
}

int DeflateStream::available() const
{
	T_ASSERT (0);
	return 0;
}

int DeflateStream::seek(SeekOriginType origin, int offset)
{
	T_ASSERT (0);
	return 0;
}

int DeflateStream::read(void* block, int nbytes)
{
	T_ASSERT (0);
	return 0;
}

int DeflateStream::write(const void* block, int nbytes)
{
	T_ASSERT (m_impl);
	return m_impl->write(block, nbytes);
}

void DeflateStream::flush()
{
	T_ASSERT (m_impl);
	m_impl->flush();
}

	}
}
