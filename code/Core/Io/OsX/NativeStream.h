#pragma once

#include <cstdio>
#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! macOS native file stream.
 * \ingroup Core
 */
class T_DLLCLASS NativeStream : public IStream
{
	T_RTTI_CLASS;

public:
	NativeStream(std::FILE* fp, uint32_t mode);

	virtual ~NativeStream();

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
	std::FILE* m_fp;
	uint32_t m_mode;
	size_t m_fileSize;
};

}

