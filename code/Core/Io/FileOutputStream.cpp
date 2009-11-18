#include "Core/Io/FileOutputStream.h"
#include "Core/Io/IEncoding.h"
#include "Core/Io/IOutputStreamBuffer.h"
#include "Core/Io/IStream.h"

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
	std::vector< uint8_t > m_encoded;
};

FileOutputStreamBuffer::FileOutputStreamBuffer(IStream* stream, IEncoding* encoding)
:	m_stream(stream)
,	m_encoding(encoding)
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
	m_encoded.resize(count * IEncoding::MaxEncodingSize);
	
	int encodedCount = m_encoding->translate(buffer, count, &m_encoded[0]);
	if (encodedCount < 0)
		return -1;

	if (m_stream->write(&m_encoded[0], encodedCount) != encodedCount)
		return -1;

	return count;
}

	}

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
