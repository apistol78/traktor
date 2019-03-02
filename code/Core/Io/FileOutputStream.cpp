#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileOutputStreamBuffer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.FileOutputStream", FileOutputStream, OutputStream)

FileOutputStream::FileOutputStream(IStream* stream, IEncoding* encoding, LineEnd lineEnd)
:	OutputStream(new FileOutputStreamBuffer(stream, encoding), lineEnd)
{
}

void FileOutputStream::close()
{
	static_cast< FileOutputStreamBuffer* >(getBuffer())->close();
}

}
