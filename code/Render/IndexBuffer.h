#ifndef traktor_render_IndexBuffer_H
#define traktor_render_IndexBuffer_H

#include "Core/Object.h"
#include "Render/Types.h"

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

/*! \brief Index buffer.
 * \ingroup Render
 *
 * Index buffer base class, each renderer should create
 * a generalized version of this class.
 */
class T_DLLCLASS IndexBuffer : public Object
{
	T_RTTI_CLASS;

public:
	IndexBuffer(IndexType indexType, uint32_t bufferSize);

	IndexType getIndexType() const;

	uint32_t getBufferSize() const;

	virtual void destroy() = 0;

	virtual void* lock() = 0;
	
	virtual void unlock() = 0;

private:
	IndexType m_indexType;
	uint32_t m_bufferSize;
};
	
	}
}

#endif	// traktor_render_IndexBuffer_H
