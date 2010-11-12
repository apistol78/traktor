#include <cell/cell_fs.h>
#include "Core/Io/File.h"
#include "Core/Io/Ps3/NativeStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeStream", NativeStream, IStream)

NativeStream::NativeStream(int32_t fd, uint32_t mode)
:	m_fd(fd)
,	m_mode(mode)
,	m_fileSize(0)
{
	if (m_mode == File::FmRead)
	{
		CellFsStat sb;
		if (cellFsFstat(m_fd, &sb) == CELL_FS_SUCCEEDED)
			m_fileSize = sb.st_size;
	}
}

NativeStream::~NativeStream()
{
	close();
}

void NativeStream::close()
{
	if (m_fd)
	{
		flush();
		cellFsClose(m_fd);
		m_fd = 0;
	}
}

bool NativeStream::canRead() const
{
	return (m_fd != 0 && m_mode == File::FmRead);
}

bool NativeStream::canWrite() const
{
	return (m_fd != 0 && (m_mode == File::FmWrite || m_mode == File::FmAppend));
}

bool NativeStream::canSeek() const
{
	return (m_fd != 0);
}

int NativeStream::tell() const
{
	if (m_fd == 0)
		return 0;

	uint64_t pos;
	if (cellFsLseek(m_fd, 0, CELL_FS_SEEK_CUR, &pos) == CELL_FS_SUCCEEDED)
		return int(pos);
	else
		return 0;
}

int NativeStream::available() const
{
	return (m_fd != 0) ? ((int)m_fileSize - tell()) : 0;
}

int NativeStream::seek(SeekOriginType origin, int offset)
{
	if (m_fd == 0)
		return 0;

	const int whence[] = { CELL_FS_SEEK_CUR, CELL_FS_SEEK_END, CELL_FS_SEEK_SET };
	uint64_t pos;

	if (cellFsLseek(m_fd, offset, whence[origin], &pos) == CELL_FS_SUCCEEDED)
		return int(pos);
	else
		return 0;
}

int NativeStream::read(void* block, int nbytes)
{
	if (m_fd == 0)
		return 0;

	uint64_t nread;
	if (cellFsRead(m_fd, block, nbytes, &nread) == CELL_FS_SUCCEEDED)
		return int(nread);
	else
		return 0;
}

int NativeStream::write(const void* block, int nbytes)	
{
	if (m_fd == 0)
		return 0;

	uint64_t nwritten;
	if (cellFsWrite(m_fd, block, nbytes, &nwritten) == CELL_FS_SUCCEEDED)
		return int(nwritten);
	else
		return 0;
}

void NativeStream::flush()
{
	if (m_fd != 0 && (m_mode & (File::FmWrite | File::FmAppend)) != 0)
		cellFsFsync(m_fd);
}

}
