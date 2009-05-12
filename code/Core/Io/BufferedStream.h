#ifndef traktor_BufferedStream_H
#define traktor_BufferedStream_H

#include "Core/Heap/Ref.h"
#include "Core/Io/Stream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
class T_DLLCLASS BufferedStream : public Stream
{
	T_RTTI_CLASS(BufferedStream)

public:
	BufferedStream(Stream* stream, uint32_t internalBufferSize = 4096);

	virtual ~BufferedStream();

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
	Ref< Stream > m_stream;
	uint32_t m_internalBufferSize;
	uint8_t* m_readBuf;
	int m_readBufCnt[2];
	uint8_t* m_writeBuf;
	int m_writeBufCnt;
};

}

#endif	// traktor_BufferedStream_H
