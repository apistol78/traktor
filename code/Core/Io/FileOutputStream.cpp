#include "Core/Io/FileOutputStream.h"
#include "Core/Io/OutputStreamBuffer.h"
#include "Core/Io/Stream.h"
#include "Core/Io/Encoding.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace
	{

class T_DLLCLASS FileOutputStreamBuffer : public OutputStreamBuffer
{
	T_RTTI_CLASS(FileOutputStreamBuffer)

public:
	FileOutputStreamBuffer(Stream* stream, Encoding* encoding);

	void close();

protected:
	virtual int overflow(const wchar_t* buffer, int count);

private:
	Ref< Stream > m_stream;
	Ref< Encoding > m_encoding;
	std::vector< uint8_t > m_encoded;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.FileOutputStreamBuffer", FileOutputStreamBuffer, OutputStreamBuffer)

FileOutputStreamBuffer::FileOutputStreamBuffer(Stream* stream, Encoding* encoding)
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
	m_encoded.resize(count * Encoding::MaxEncodingSize);
	
	int encodedCount = m_encoding->translate(buffer, count, &m_encoded[0]);
	if (encodedCount < 0)
		return -1;

	if (m_stream->write(&m_encoded[0], encodedCount) != encodedCount)
		return -1;

	return count;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.FileOutputStream", FileOutputStream, OutputStream)

FileOutputStream::FileOutputStream(Stream* stream, Encoding* encoding)
:	OutputStream(gc_new< FileOutputStreamBuffer >(stream, encoding))
{
}

void FileOutputStream::close()
{
	flush();
	checked_type_cast< FileOutputStreamBuffer* >(getBuffer())->close();
}

}
