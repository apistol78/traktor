#ifndef traktor_editor_MemCachedPutStream_H
#define traktor_editor_MemCachedPutStream_H

#include "Core/Io/IStream.h"

namespace traktor
{
	namespace editor
	{

class MemCachedProto;

class MemCachedPutStream : public IStream
{
	T_RTTI_CLASS;

public:
	enum { MaxBlockSize = 65536 };

	MemCachedPutStream(MemCachedProto* proto, const std::string& key);

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
	Ref< MemCachedProto > m_proto;
	std::string m_key;
	uint8_t m_block[MaxBlockSize + 2];
	uint32_t m_inblock;
	uint32_t m_index;

	bool uploadBlock();
};

	}
}

#endif	// traktor_editor_MemCachedPutStream_H
