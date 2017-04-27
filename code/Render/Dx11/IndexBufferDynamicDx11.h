/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IndexBufferDynamicDx11_H
#define traktor_render_IndexBufferDynamicDx11_H

#include "Render/Dx11/IndexBufferDx11.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*!
 * \ingroup DX11
 */
class IndexBufferDynamicDx11 : public IndexBufferDx11
{
	T_RTTI_CLASS;

public:
	static Ref< IndexBufferDynamicDx11 > create(ContextDx11* context, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferDynamicDx11();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void prepare(ID3D11DeviceContext* d3dDeviceContext, StateCache& stateCache) T_OVERRIDE T_FINAL;

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11DeviceContext > m_d3dDeferredContext;
	ComRef< ID3D11CommandList > m_d3dPendingCommandList;
	Semaphore m_lock;

	IndexBufferDynamicDx11(IndexType indexType, uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_IndexBufferDynamicDx11_H
