#include "Zip/StreamUnzip.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace zip
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.zip.StreamUnzip", StreamUnzip, IStream)

StreamUnzip::StreamUnzip(unzFile file) :
	m_file(file)
{
	unzGetCurrentFileInfo(m_file, &m_info, 0, 0, 0, 0, 0, 0);
}

void StreamUnzip::close()
{
	if (unzCloseCurrentFile(m_file) == UNZ_CRCERROR)
		log::warning << L"CRC error in archive";
}

bool StreamUnzip::canRead() const
{
	return true;
}

bool StreamUnzip::canWrite() const
{
	return false;
}

bool StreamUnzip::canSeek() const
{
	return false;
}

int StreamUnzip::tell() const
{
	return unztell(m_file);
}

int StreamUnzip::available() const
{
	return m_info.uncompressed_size - tell();
}

int StreamUnzip::seek(SeekOriginType origin, int offset)
{
	switch (origin)
	{
	case SeekCurrent:
		{
			uLong currentOffset = unzGetOffset(m_file);
			unzSetOffset(m_file, currentOffset + offset);
		}
		break;

	case SeekEnd:
		unzSetOffset(m_file, m_info.uncompressed_size + offset);
		break;

	case SeekSet:
		unzSetOffset(m_file, offset);
		break;
	}
	return offset;
}

int StreamUnzip::read(void* block, int nbytes)
{
	int nread = std::min< int >(available(), nbytes);
	return unzReadCurrentFile(m_file, block, nread);
}

int StreamUnzip::write(const void* block, int nbytes)
{
	return 0;
}

void StreamUnzip::flush()
{
}
	
	}
}
