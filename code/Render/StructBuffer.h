#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! Struct buffer.
 * \ingroup Render
 */
class T_DLLCLASS StructBuffer : public Object
{
	T_RTTI_CLASS;

public:
	/*! Get buffer size in bytes.
	 *
	 * \return Buffer size.
	 */
	uint32_t getBufferSize() const;

	/*! Destroy resources allocated by this buffer. */
	virtual void destroy() = 0;

	/*! Lock access to entire buffer data.
	 *
	 * \return Pointer to buffer beginning.
	 */
	virtual void* lock() = 0;

	/*! Lock access to segment of buffer data.
	 *
	 * \param structOffset Struct offset.
	 * \param structCount Number of structs to lock.
	 * \return Pointer to segment beginning.
	 */
	virtual void* lock(uint32_t structOffset, uint32_t structCount) = 0;

	/*! Unlock access. */
	virtual void unlock() = 0;

protected:
	StructBuffer(uint32_t bufferSize);

private:
	uint32_t m_bufferSize;
};

	}
}

