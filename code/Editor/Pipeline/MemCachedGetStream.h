#ifndef traktor_editor_MemCachedGetStream_H
#define traktor_editor_MemCachedGetStream_H

#include "Core/Io/IStream.h"

namespace traktor
{
	namespace editor
	{

class MemCachedProto;

class MemCachedGetStream : public IStream
{
	T_RTTI_CLASS;

public:
	enum { MaxBlockSize = 65536 };

	MemCachedGetStream(MemCachedProto* proto, const std::string& key);

	bool requestNextBlock();

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
	uint32_t m_index;
	int32_t m_inblock;
	uint8_t m_block[MaxBlockSize + 2];
};


	}
}

#endif	// traktor_editor_MemCachedGetStream_H
