#pragma once

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
		const AlignedVector< VertexElement >& vertexElements
	);

	virtual ~VertexBufferDynamicDx11();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) override final;

	virtual void unlock() override final;

	virtual void prepare(ID3D11DeviceContext* d3dDeviceContext, StateCache& stateCache) override final;

private:
	Ref< ContextDx11 > m_context;
	AutoArrayPtr< uint8_t, AllocFreeAlign > m_data;
	bool m_dirty;

	VertexBufferDynamicDx11(uint32_t bufferSize);
};

	}
}

