#include <cell/cell_fs.h>
#include "Core/Io/File.h"
#include "Core/Io/Ps3/NativeStream.h"

namespace traktor
{
	namespace
	{

const int c_pageType = CELL_FS_IO_BUFFER_PAGE_SIZE_1MB;
const size_t c_containerSize = 1024 * 1024;
const size_t c_bufferSize = 1024 * 1024;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeStream", NativeStream, IStream)

NativeStream::NativeStream(int32_t fd, uint32_t mode)
:	m_fd(fd)
,	m_mode(mode)
,	m_fileSize(0)
,	m_container(SYS_MEMORY_CONTAINER_ID_INVALID)
{
	if ((m_mode & File::FmRead) != 0)
	{
		CellFsStat sb;
		if (cellFsFstat(m_fd, &sb) == CELL_FS_SUCCEEDED)
			m_fileSize = sb.st_size;

		int32_t err = sys_memory_container_create(&m_container, c_containerSize);
		if (err == CELL_OK)
			cellFsSetIoBuffer(m_fd, c_bufferSize, c_pageType, m_container);
		else
			m_container = SYS_MEMORY_CONTAINER_ID_INVALID;
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

	if (m_container != SYS_MEMORY_CONTAINER_ID_INVALID)
	{
		sys_memory_container_destroy(m_container);
		m_container = SYS_MEMORY_CONTAINER_ID_INVALID;
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

int64_t NativeStream::tell() const
{
	if (m_fd == 0)
		return 0;

	uint64_t pos;
	if (cellFsLseek(m_fd, 0, CELL_FS_SEEK_CUR, &pos) == CELL_FS_SUCCEEDED)
		return int64_t(pos);
	else
		return 0;
}

int64_t NativeStream::available() const
{
	return (m_fd != 0) ? ((int64_t)m_fileSize - tell()) : 0;
}

int64_t NativeStream::seek(SeekOriginType origin, int64_t offset)
{
	if (m_fd == 0)
		return 0;

	const int whence[] = { CELL_FS_SEEK_CUR, CELL_FS_SEEK_END, CELL_FS_SEEK_SET };
	uint64_t pos;

	if (cellFsLseek(m_fd, offset, whence[origin], &pos) == CELL_FS_SUCCEEDED)
		return int64_t(pos);
	else
		return 0;
}

int64_t NativeStream::read(void* block, int64_t nbytes)
{
	if (m_fd == 0)
		return 0;

	uint64_t nread;
	if (cellFsRead(m_fd, block, nbytes, &nread) == CELL_FS_SUCCEEDED)
		return int64_t(nread);
	else
		return 0;
}

int64_t NativeStream::write(const void* block, int64_t nbytes)	
{
	if (m_fd == 0)
		return 0;

	uint64_t nwritten;
	if (cellFsWrite(m_fd, block, nbytes, &nwritten) == CELL_FS_SUCCEEDED)
		return int64_t(nwritten);
	else
		return 0;
}

void NativeStream::flush()
{
	if (m_fd != 0 && (m_mode & (File::FmWrite | File::FmAppend)) != 0)
		cellFsFsync(m_fd);
}

}
