#include "Core/Io/FileSystem.h"
#include "Editor/Pipeline/FilePipelinePutStream.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.FilePipelinePutStream", FilePipelinePutStream, IStream)

FilePipelinePutStream::FilePipelinePutStream(IStream* file, const std::wstring& path)
:	m_file(file)
,	m_path(path)
{
}

void FilePipelinePutStream::close()
{
	if (m_file)
	{
		m_file->close();
		m_file = 0;
		
		FileSystem::getInstance().move(
			m_path,
			m_path + L"~"
		);
	}
}

bool FilePipelinePutStream::canRead() const
{
	return m_file->canRead();
}

bool FilePipelinePutStream::canWrite() const
{
	return m_file->canWrite();
}

bool FilePipelinePutStream::canSeek() const
{
	return m_file->canSeek();
}

int FilePipelinePutStream::tell() const
{
	return m_file->tell();
}

int FilePipelinePutStream::available() const
{
	return m_file->available();
}

int FilePipelinePutStream::seek(SeekOriginType origin, int offset)
{
	return m_file->seek(origin, offset);
}

int FilePipelinePutStream::read(void* block, int nbytes)
{
	return m_file->read(block, nbytes);
}

int FilePipelinePutStream::write(const void* block, int nbytes)
{
	return m_file->write(block, nbytes);
}

void FilePipelinePutStream::flush()
{
	return m_file->flush();
}
	
	}
}
