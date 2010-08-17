#ifndef traktor_editor_FilePipelinePutStream_H
#define traktor_editor_FilePipelinePutStream_H

#include "Core/Io/IStream.h"

namespace traktor
{
	namespace editor
	{

class FilePipelinePutStream : public IStream
{
	T_RTTI_CLASS;

public:
	FilePipelinePutStream(IStream* file, const std::wstring& path);

	virtual void close();

	virtual bool canRead() const;

	virtual bool canWrite() const;

	virtual bool canSeek() const;

	virtual int tell() const;

	virtual int available() const;

	virtual int seek(SeekOriginType origin, int offset);

	virtual int read(void* block, int nbytes);

	virtual int write(const void* block, int nbytes);

	virtual void flush();

private:
	Ref< IStream > m_file;
	std::wstring m_path;
};

	}
}

#endif	// traktor_editor_FilePipelinePutStream_H
