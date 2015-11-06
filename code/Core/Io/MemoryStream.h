#ifndef traktor_MemoryStream_H
#define traktor_MemoryStream_H

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

/*! \brief Fixed memory stream.
 * \ingroup Core
 */
class T_DLLCLASS MemoryStream : public IStream
{
	T_RTTI_CLASS;

public:
	MemoryStream(void* buffer, uint32_t bufferSize, bool readAllowed = true, bool writeAllowed = true, bool own = false);

	MemoryStream(const void* buffer, uint32_t bufferSize);
	
	virtual ~MemoryStream();

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
	uint8_t* m_buffer;
	uint8_t* m_bufferPtr;
	uint32_t m_bufferSize;
	bool m_readAllowed;
	bool m_writeAllowed;
	bool m_own;
};

}

#endif	// traktor_MemoryStream_H
