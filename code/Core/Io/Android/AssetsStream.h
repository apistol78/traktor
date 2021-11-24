#pragma once

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
	AAsset* m_assetFile;
	uint32_t m_size;
};

}

