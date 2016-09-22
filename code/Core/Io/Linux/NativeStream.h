#ifndef traktor_NativeStream_H
#define traktor_NativeStream_H

#include <cstdio>
#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#   define T_DLLCLASS T_DLLEXPORT
#else
#   define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class T_DLLCLASS NativeStream : public IStream
{
	T_RTTI_CLASS;

public:
	NativeStream(std::FILE* fp, uint32_t mode);

	virtual ~NativeStream();

	virtual void close() T_OVERRIDE T_FINAL;

	virtual bool canRead() const T_OVERRIDE T_FINAL;

	virtual bool canWrite() const T_OVERRIDE T_FINAL;

	virtual bool canSeek() const T_OVERRIDE T_FINAL;

	virtual int tell() const T_OVERRIDE T_FINAL;

	virtual int available() const T_OVERRIDE T_FINAL;

	virtual int seek(SeekOriginType origin, int offset) T_OVERRIDE T_FINAL;

	virtual int read(void* block, int nbytes) T_OVERRIDE T_FINAL;

	virtual int write(const void* block, int nbytes) T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

private:
	std::FILE* m_fp;
	uint32_t m_mode;
	size_t m_fileSize;
};

}

#endif	// traktor_NativeStream_H
