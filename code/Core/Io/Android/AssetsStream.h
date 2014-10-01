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
	AAsset* m_assetFile;
	uint32_t m_size;
};

}

#endif	// traktor_AssetsStream_H
