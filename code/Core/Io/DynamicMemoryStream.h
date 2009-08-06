#ifndef traktor_DynamicMemoryStream_H
#define traktor_DynamicMemoryStream_H

#include <vector>
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

/*! \brief Dynamic memory stream wrapper.
 * \ingroup Core
 */
class T_DLLCLASS DynamicMemoryStream : public Stream
{
	T_RTTI_CLASS(DynamicMemoryStream)

public:
	DynamicMemoryStream(std::vector< uint8_t >& buffer, bool readAllowed = true, bool writeAllowed = true);

	DynamicMemoryStream(bool readAllowed = true, bool writeAllowed = true);

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

	const std::vector< uint8_t >& getBuffer() const;

private:
	std::vector< uint8_t > m_internal;
	std::vector< uint8_t >& m_buffer;
	std::vector< uint8_t >::iterator m_bufferIter;
	bool m_readAllowed;
	bool m_writeAllowed;
};

}

#endif	// traktor_DynamicMemoryStream_H
