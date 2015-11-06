#ifndef traktor_BufferedStream_H
#define traktor_BufferedStream_H

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
	Ref< IStream > m_stream;
	uint32_t m_internalBufferSize;
	AutoArrayPtr< uint8_t > m_readBuf;
	AutoArrayPtr< uint8_t > m_writeBuf;
	int32_t m_readBufCnt[2];
	int32_t m_writeBufCnt;

	void flushWriteBuffer();
};

}

#endif	// traktor_BufferedStream_H
