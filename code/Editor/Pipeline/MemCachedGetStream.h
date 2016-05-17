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
	
	bool requestEndBlock();

	bool requestNextBlock();

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
	Ref< MemCachedProto > m_proto;
	std::string m_key;
	uint32_t m_index;
	int32_t m_inblock;
	uint8_t m_block[MaxBlockSize + 2];
};

	}
}

#endif	// traktor_editor_MemCachedGetStream_H
