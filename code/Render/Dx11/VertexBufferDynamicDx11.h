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

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);
	
	virtual void unlock();

	virtual void prepare(ID3D11DeviceContext* d3dDeviceContext);

private:
	Ref< ContextDx11 > m_context;
	AutoArrayPtr< uint8_t > m_data;
	bool m_dirty;

	VertexBufferDynamicDx11(uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_VertexBufferDynamicDx11_H
