#include "Zip/StreamZip.h"

namespace traktor
{
	namespace zip
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.zip.StreamZip", StreamZip, Stream)

StreamZip::StreamZip(zipFile file) :
	m_file(file)
{
}

void StreamZip::close()
{
	zipCloseFileInZip(m_file);
}

bool StreamZip::canRead() const
{
	return false;
}

bool StreamZip::canWrite() const
{
	return true;
}

bool StreamZip::canSeek() const
{
	return false;
}

int StreamZip::tell() const
{
	return 0;
}

int StreamZip::available() const
{
	return 0;
}

int StreamZip::seek(SeekOriginType origin, int offset)
{
	return 0;
}

int StreamZip::read(void* block, int nbytes)
{
	return 0;
}

int StreamZip::write(const void* block, int nbytes)
{
	if (zipWriteInFileInZip(m_file, block, nbytes) != ZIP_OK)
		return 0;
	return nbytes;
}

void StreamZip::flush()
{
}
	
	}
}
