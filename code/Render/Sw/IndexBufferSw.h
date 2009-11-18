#ifndef traktor_render_IndexBufferSw_H
#define traktor_render_IndexBufferSw_H

#include <vector>
#include "Render/IndexBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_SW_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup SW
 */
class T_DLLCLASS IndexBufferSw : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferSw(IndexType indexType, uint32_t bufferSize);

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	const std::vector< uint32_t >& getIndices() const;
	
private:
	std::vector< uint32_t > m_indices;
	void* m_lock;
};
	
	}
}

#endif	// traktor_render_IndexBufferSw_H
