#ifndef traktor_render_IndexBufferStaticDx11_H
#define traktor_render_IndexBufferStaticDx11_H

#include "Core/Misc/AutoPtr.h"
#include "Render/Dx11/IBufferHeapDx11.h"
#include "Render/Dx11/IndexBufferDx11.h"

namespace traktor
{
	namespace render
	{

class IBufferHeapDx11;
class ContextDx11;

/*!
 * \ingroup DX11
 */
class IndexBufferStaticDx11 : public IndexBufferDx11
{
	T_RTTI_CLASS;

public:
	static Ref< IndexBufferStaticDx11 > create(ContextDx11* context, IBufferHeapDx11* bufferHeap, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferStaticDx11();

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	virtual void prepare(ID3D11DeviceContext* d3dDeviceContext, StateCache& stateCache);

private:
	Ref< ContextDx11 > m_context;
	Ref< IBufferHeapDx11 > m_bufferHeap;
	IBufferHeapDx11::Chunk m_bufferChunk;
	AutoArrayPtr< uint8_t > m_data;

	IndexBufferStaticDx11(IndexType indexType, uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_IndexBufferStaticDx11_H
