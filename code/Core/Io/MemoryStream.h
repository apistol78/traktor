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
	MemoryStream(void* buffer, uint32_t bufferSize, bool readAllowed = true, bool writeAllowed = true);

	MemoryStream(const void* buffer, uint32_t bufferSize);

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
	uint8_t* m_buffer;
	uint8_t* m_bufferPtr;
	uint32_t m_bufferSize;
	bool m_readAllowed;
	bool m_writeAllowed;
};

}

#endif	// traktor_MemoryStream_H
