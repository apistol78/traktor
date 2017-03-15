#ifndef traktor_AssetsStream_H
#define traktor_AssetsStream_H

#include "Core/Io/IStream.h"

struct AAsset;

namespace traktor
{

class AssetsStream : public IStream
{
	T_RTTI_CLASS;

public:
	AssetsStream(AAsset* assetFile);

	virtual ~AssetsStream();

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
	AAsset* m_assetFile;
	uint32_t m_size;
};

}

#endif	// traktor_AssetsStream_H
