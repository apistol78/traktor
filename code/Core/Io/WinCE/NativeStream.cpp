#include "Core/Io/WinCE/NativeStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeStream", NativeStream, IStream)

NativeStream::NativeStream(HANDLE hFile, uint32_t mode)
:	m_hFile(hFile)
,	m_mode(mode)
,	m_fileSize(0)
{
	if (m_mode & File::FmRead)
		m_fileSize = GetFileSize(m_hFile, NULL);
}

NativeStream::~NativeStream()
{
	close();
}

void NativeStream::close()
{
	if (m_hFile)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
	}
}

bool NativeStream::canRead() const
{
	return m_hFile != 0 && (m_mode & File::FmRead) == File::FmRead;
}

bool NativeStream::canWrite() const
{
	return m_hFile != 0 && ((m_mode & File::FmWrite) == File::FmWrite || (m_mode & File::FmAppend) == File::FmAppend);
}

bool NativeStream::canSeek() const
{
	return m_hFile != 0;
}

int NativeStream::tell() const
{
	return m_hFile != 0 ? (int)SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT) : 0;
}

int NativeStream::available() const
{
	return m_hFile != 0 ? ((int)m_fileSize - tell()) : 0;
}

int NativeStream::seek(SeekOriginType origin, int offset)
{
	if (m_hFile == 0)
		return 0;

	const DWORD c_origins[] = { FILE_CURRENT, FILE_END, FILE_BEGIN };
	return (int)SetFilePointer(m_hFile, offset, NULL, c_origins[origin]);
}

int NativeStream::read(void* block, int nbytes)
{
	if (m_hFile == 0)
		return 0;

	DWORD nread = 0;
	ReadFile(
		m_hFile,
		block,
		nbytes,
		&nread,
		NULL
	);

	return int(nread);
}

int NativeStream::write(const void* block, int nbytes)	
{
	if (m_hFile == 0)
		return 0;

	DWORD nwritten = 0;
	WriteFile(
		m_hFile,
		block,
		nbytes,
		&nwritten,
		NULL
	);

	return int(nwritten);
}

void NativeStream::flush()
{
	if (m_hFile != 0)
		FlushFileBuffers(m_hFile);
}

}
