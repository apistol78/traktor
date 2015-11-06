#ifndef traktor_DebugStream_H
#define traktor_DebugStream_H

#include "Core/Io/IStream.h"
#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class OutputStream;

/*! \brief Debug stream wrapper.
 * \ingroup Core
 *
 * Measure number of calls made to a stream.
 * At any given time it's possible to request
 * a report by calling the "dump" method.
 */
class T_DLLCLASS DebugStream : public IStream
{
	T_RTTI_CLASS;

public:
	DebugStream(IStream* stream);

	virtual ~DebugStream();

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

	void dump(OutputStream& os) const;

private:
	Ref< IStream > m_stream;
	mutable uint32_t m_canReadCalls;
	mutable uint32_t m_canWriteCalls;
	mutable uint32_t m_canSeekCalls;
	mutable uint32_t m_tellCalls;
	mutable uint32_t m_availableCalls;
	mutable uint32_t m_seekCalls;
	mutable uint32_t m_readCalls;
	mutable uint32_t m_writeCalls;
	mutable uint32_t m_flushCalls;
	mutable uint32_t m_readTotals[8];
	mutable uint32_t m_readTotal;
	mutable uint32_t m_writeTotals[8];
	mutable uint32_t m_writeTotal;
	mutable double m_readIoTime;
	mutable double m_writeIoTime;
	mutable double m_flushIoTime;
	mutable Timer m_timer;
};

}

#endif	// traktor_DebugStream_H
