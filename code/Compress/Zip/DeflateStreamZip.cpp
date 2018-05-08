/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include <zlib.h>
#include "Compress/Zip/DeflateStreamZip.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Memory/Alloc.h"

namespace traktor
{
	namespace compress
	{
		namespace
		{
		
voidpf deflateZAlloc(voidpf opaque, uInt items, uInt size)
{
	return Alloc::acquire(items * size, "zlib");
}

void deflateZFree(voidpf opaque, voidpf address)
{
	Alloc::free(address);
}
		
		}

class DeflateZipImpl : public RefCountImpl< IRefCount >
{
public:
	DeflateZipImpl(IStream* stream, uint32_t internalBufferSize)
	:	m_stream(stream)
	,	m_buf(internalBufferSize)
	,	m_position(stream->tell())
	{
		std::memset(&m_zstream, 0, sizeof(m_zstream));

		m_zstream.zalloc = &deflateZAlloc;
		m_zstream.zfree = &deflateZFree;

		int rc = deflateInit(&m_zstream, Z_DEFAULT_COMPRESSION);
		T_FATAL_ASSERT (rc == Z_OK);

		m_zstream.next_out = (Bytef*)&m_buf[0];
		m_zstream.avail_out = (uInt)m_buf.size();
	}

	void close()
	{
		flush();
		deflateEnd(&m_zstream);
		m_stream = 0;
	}

	int64_t write(const void* block, int64_t nbytes)
	{
		int rc;

		m_zstream.next_in = (Bytef*)block;
		m_zstream.avail_in = nbytes;

		for (;;)
		{
			T_ASSERT (m_zstream.avail_out > 0);
			rc = deflate(&m_zstream, Z_NO_FLUSH);
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
#if defined(_DEBUG)
			int rc =
#endif
			deflate(&m_zstream, Z_FULL_FLUSH);
			T_ASSERT (rc == Z_OK || rc == Z_BUF_ERROR);

			int nwrite = int(m_buf.size()) - m_zstream.avail_out;
			if (nwrite > 0)
			{
				T_ASSERT (nwrite <= int(m_buf.size()));
				int nwritten = m_stream->write(&m_buf[0], nwrite);
				T_ASSERT (nwritten == nwrite);
				nwritten = nwrite;
			}

			if (m_zstream.avail_out > 0)
				break;

			m_zstream.next_out = (Bytef*)&m_buf[0];
			m_zstream.avail_out = (uInt)m_buf.size();
		}
	}

	int64_t getLogicalPosition() const
	{
		return m_position;
	}

private:
	Ref< IStream > m_stream;
	z_stream m_zstream;
	AlignedVector< uint8_t > m_buf;
	int64_t m_position;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.compress.DeflateStreamZip", DeflateStreamZip, IStream)

DeflateStreamZip::DeflateStreamZip(IStream* stream, uint32_t internalBufferSize)
:	m_impl(new DeflateZipImpl(stream, internalBufferSize))
{
}

DeflateStreamZip::~DeflateStreamZip()
{
	close();
}

void DeflateStreamZip::close()
{
	if (m_impl)
	{
		m_impl->close();
		m_impl = 0;
	}
}

bool DeflateStreamZip::canRead() const
{
	return false;
}

bool DeflateStreamZip::canWrite() const
{
	return true;
}

bool DeflateStreamZip::canSeek() const
{
	return false;
}

int64_t DeflateStreamZip::tell() const
{
	T_ASSERT (m_impl);
	return m_impl->getLogicalPosition();
}

int64_t DeflateStreamZip::available() const
{
	T_ASSERT (0);
	return 0;
}

int64_t DeflateStreamZip::seek(SeekOriginType origin, int64_t offset)
{
	T_ASSERT (0);
	return 0;
}

int64_t DeflateStreamZip::read(void* block, int64_t nbytes)
{
	T_ASSERT (0);
	return 0;
}

int64_t DeflateStreamZip::write(const void* block, int64_t nbytes)
{
	T_ASSERT (m_impl);
	return m_impl->write(block, nbytes);
}

void DeflateStreamZip::flush()
{
	T_ASSERT (m_impl);
	m_impl->flush();
}

	}
}
