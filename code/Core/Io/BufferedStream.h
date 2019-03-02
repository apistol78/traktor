#pragma once

#include "Core/Ref.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/AutoPtr.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Buffered stream wrapper.
 * \ingroup Core
 *
 * Read from underlying stream with a fixed
 * buffer size.
 * This class is designed to improve performance
 * on slow streams by sacrificing local memory.
 */
class T_DLLCLASS BufferedStream : public IStream
{
	T_RTTI_CLASS;

public:
	enum { DefaultBufferSize = 4096 };

	BufferedStream(IStream* stream, uint32_t internalBufferSize = DefaultBufferSize);

	BufferedStream(IStream* stream, const void* appendData, uint32_t appendDataSize, uint32_t internalBufferSize = DefaultBufferSize);

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
	Ref< IStream > m_stream;
	uint32_t m_internalBufferSize;
	AutoArrayPtr< uint8_t > m_readBuf;
	AutoArrayPtr< uint8_t > m_writeBuf;
	int64_t m_readBufCnt[2];
	int64_t m_writeBufCnt;

	void flushWriteBuffer();
};

}

