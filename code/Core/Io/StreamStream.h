#ifndef traktor_StreamStream_H
#define traktor_StreamStream_H

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

/*! \brief Stream in stream.
 * \ingroup Core
 */
class T_DLLCLASS StreamStream : public Stream
{
	T_RTTI_CLASS(StreamStream)

public:
	StreamStream(Stream* stream, int endOffset = -1);

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
	int m_startOffset;
	int m_endOffset;
};

}

#endif	// traktor_StreamStream_H
