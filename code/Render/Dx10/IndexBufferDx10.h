#ifndef traktor_render_IndexBufferDx10_H
#define traktor_render_IndexBufferDx10_H

#include "Render/IndexBuffer.h"
#include "Core/Heap/Ref.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX10_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx10;

/*!
 * \ingroup DX10
 */
class T_DLLCLASS IndexBufferDx10 : public IndexBuffer
{
	T_RTTI_CLASS(IndexBufferDx10)

public:
	IndexBufferDx10(ContextDx10* context, IndexType indexType, uint32_t bufferSize, ID3D10Buffer* d3dBuffer);

	virtual ~IndexBufferDx10();

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	ID3D10Buffer* getD3D10Buffer() const;

private:
	Ref< ContextDx10 > m_context;
	ComRef< ID3D10Buffer > m_d3dBuffer;
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_IndexBufferDx10_H
