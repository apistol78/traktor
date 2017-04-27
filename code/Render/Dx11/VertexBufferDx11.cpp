/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Dx11/StateCache.h"
#include "Render/Dx11/VertexBufferDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDx11", VertexBufferDx11, VertexBuffer)

void VertexBufferDx11::prepare(ID3D11DeviceContext* d3dDeviceContext, StateCache& stateCache)
{
	stateCache.setVertexBuffer(m_d3dBuffer, m_d3dStride);
}

VertexBufferDx11::VertexBufferDx11(uint32_t bufferSize)
:	VertexBuffer(bufferSize)
,	m_d3dStride(0)
,	m_d3dBaseVertexOffset(0)
,	m_d3dInputElementsHash(0)
{
}

	}
}
