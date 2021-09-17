#pragma once

#include "Core/Ref.h"
#include "Core/Io/IStream.h"

namespace traktor
{
	namespace editor
	{

class MemcachedPipelineCache;
class MemcachedProto;

class MemcachedPutStream : public IStream
{
	T_RTTI_CLASS;

public:
	enum { MaxBlockSize = 65536 };

	MemcachedPutStream(MemcachedPipelineCache* cache, MemcachedProto* proto, const std::string& key);

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
	Ref< MemcachedPipelineCache > m_cache;
	Ref< MemcachedProto > m_proto;
	std::string m_key;
	uint8_t m_block[MaxBlockSize + 2];
	uint32_t m_inblock;
	uint32_t m_index;

	bool uploadBlock();

	void uploadEndBlock();
};

	}
}

