#ifndef traktor_StreamStream_H
#define traktor_StreamStream_H

#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Stream in stream.
 * \ingroup Core
 */
class T_DLLCLASS StreamStream : public IStream
{
	T_RTTI_CLASS;

public:
	StreamStream(IStream* stream, int endOffset = -1);

	virtual void close() T_OVERRIDE;

	virtual bool canRead() const T_OVERRIDE T_FINAL;

	virtual bool canWrite() const T_OVERRIDE T_FINAL;

	virtual bool canSeek() const T_OVERRIDE T_FINAL;

	virtual int tell() const T_OVERRIDE T_FINAL;

	virtual int available() const T_OVERRIDE T_FINAL;

	virtual int seek(SeekOriginType origin, int offset) T_OVERRIDE T_FINAL;

	virtual int read(void* block, int nbytes) T_OVERRIDE T_FINAL;

	virtual int write(const void* block, int nbytes) T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

protected:
	Ref< IStream > m_stream;
	int m_startOffset;
	int m_endOffset;
};

}

#endif	// traktor_StreamStream_H
