#ifndef traktor_IStream_H
#define traktor_IStream_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Stream base class.
 * \ingroup Core
 *
 * Streams are used through-out the framework in order
 * to abstract stream sources.
 */
class T_DLLCLASS IStream : public Object
{
	T_RTTI_CLASS;

public:
	enum SeekOriginType
	{
		SeekCurrent,
		SeekEnd,
		SeekSet
	};

	/*! \brief Close stream.
	 *
	 * No further reading or writing is permitted
	 * to the stream.
	 */
	virtual void close() = 0;

	/*! \brief Can we read from stream. */
	virtual bool canRead() const = 0;

	/*! \brief Can we write to stream. */
	virtual bool canWrite() const = 0;

	/*! \brief Can we seek in stream. */
	virtual bool canSeek() const = 0;

	/*! \brief Get current position in stream. */
	virtual int64_t tell() const = 0;

	/*! \brief Get number of bytes available from stream. */
	virtual int64_t available() const = 0;

	/*! \brief Move current position. */
	virtual int64_t seek(SeekOriginType origin, int64_t offset) = 0;

	/*! \brief Read block from stream. */
	virtual int64_t read(void* block, int64_t nbytes) = 0;

	/*! \brief Write data to stream. */
	virtual int64_t write(const void* block, int64_t nbytes) = 0;

	/*! \brief Ensure all data has been written to stream. */
	virtual void flush() = 0;
};

}

#endif	// traktor_IStream_H
