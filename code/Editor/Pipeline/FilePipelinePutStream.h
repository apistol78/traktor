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

	virtual void close() T_OVERRIDE T_FINAL;

	virtual bool canRead() const T_OVERRIDE T_FINAL;

	virtual bool canWrite() const T_OVERRIDE T_FINAL;

	virtual bool canSeek() const T_OVERRIDE T_FINAL;

	virtual int64_t tell() const T_OVERRIDE T_FINAL;

	virtual int64_t available() const T_OVERRIDE T_FINAL;

	virtual int64_t seek(SeekOriginType origin, int64_t offset) T_OVERRIDE T_FINAL;

	virtual int64_t read(void* block, int64_t nbytes) T_OVERRIDE T_FINAL;

	virtual int64_t write(const void* block, int64_t nbytes) T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

private:
	Ref< IStream > m_file;
	std::wstring m_path;
};

	}
}

#endif	// traktor_editor_FilePipelinePutStream_H
