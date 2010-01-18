#include "Core/Io/FileOutputStream.h"
#include "Core/Io/IEncoding.h"
#include "Core/Io/IOutputStreamBuffer.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{
	namespace
	{

class FileOutputStreamBuffer : public IOutputStreamBuffer
{
public:
	FileOutputStreamBuffer(IStream* stream, IEncoding* encoding);

	void close();

	virtual int overflow(const wchar_t* buffer, int count);

private:
	Ref< IStream > m_stream;
	Ref< IEncoding > m_encoding;
	AutoArrayPtr< uint8_t > m_encoded;
	uint32_t m_encodedSize;
};

FileOutputStreamBuffer::FileOutputStreamBuffer(IStream* stream, IEncoding* encoding)
:	m_stream(stream)
,	m_encoding(encoding)
,	m_encodedSize(0)
{
}

void FileOutputStreamBuffer::close()
{
	if (m_stream)
	{
		m_stream->close();
		m_stream = 0;
	}
}

int FileOutputStreamBuffer::overflow(const wchar_t* buffer, int count)
{
	T_ASSERT (count > 0);

	uint32_t maxEncodedSize = count * IEncoding::MaxEncodingSize;
	if (maxEncodedSize > m_encodedSize)
	{
		m_encodedSize = maxEncodedSize;
		m_encoded.reset(new uint8_t [m_encodedSize]);
	}
	
	int encodedCount = m_encoding->translate(buffer, count, m_encoded.ptr());
	if (encodedCount < 0)
		return -1;

	if (m_stream->write(m_encoded.c_ptr(), encodedCount) != encodedCount)
		return -1;

	return count;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.FileOutputStream", FileOutputStream, OutputStream)

FileOutputStream::FileOutputStream(IStream* stream, IEncoding* encoding, LineEnd lineEnd)
:	OutputStream(new FileOutputStreamBuffer(stream, encoding), lineEnd)
{
}

void FileOutputStream::close()
{
	flush();
	static_cast< FileOutputStreamBuffer* >(getBuffer())->close();
}

}
