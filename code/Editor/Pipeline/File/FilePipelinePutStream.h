#pragma once

#include "Core/Io/IStream.h"

namespace traktor::editor
{

class FilePipelinePutStream : public IStream
{
	T_RTTI_CLASS;

public:
	FilePipelinePutStream(IStream* file, const std::wstring& path);

	virtual void close() override final;

	virtual bool canRead() const override final;

	virtual bool canWrite() const override final;

	virtual bool canSeek() const override final;

	virtual int64_t tell() const override final;

	virtual int64_t available() const override final;

	virtual int64_t seek(SeekOriginType origin, int64_t offset) override final;

	virtual int64_t read(void* block, int64_t nbytes) override final;

	virtual int64_t write(const void* block, int64_t nbytes) override final;

	virtual void flush() override final;

private:
	Ref< IStream > m_file;
	std::wstring m_path;
};

}
