#ifndef traktor_render_VertexBufferDynamicDx11_H
#define traktor_render_VertexBufferDynamicDx11_H

#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/ComRef.h"
#include "Render/VertexElement.h"
#include "Render/Dx11/VertexBufferDx11.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*!
 * \ingroup DX11
 */
class VertexBufferDynamicDx11 : public VertexBufferDx11
{
	T_RTTI_CLASS;

public:
	static Ref< VertexBufferDynamicDx11 > create(
		ContextDx11* context,
		uint32_t bufferSize,
		const std::vector< VertexElement >& vertexElements
	);

	virtual ~VertexBufferDynamicDx11();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void prepare(ID3D11DeviceContext* d3dDeviceContext, StateCache& stateCache) T_OVERRIDE T_FINAL;

private:
	Ref< ContextDx11 > m_context;
	AutoArrayPtr< uint8_t, AllocFreeAlign > m_data;
	bool m_dirty;

	VertexBufferDynamicDx11(uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_VertexBufferDynamicDx11_H
