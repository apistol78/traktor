/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexBufferDx11_H
#define traktor_render_VertexBufferDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/VertexBuffer.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

class StateCache;

/*!
 * \ingroup DX11
 */
class VertexBufferDx11 : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	virtual void prepare(ID3D11DeviceContext* d3dDeviceContext, StateCache& stateCache);

	ID3D11Buffer* getD3D11Buffer() const { return m_d3dBuffer; }

	UINT getD3D11Stride() const { return m_d3dStride; }

	UINT getD3D11BaseVertexOffset() const { return m_d3dBaseVertexOffset; }

	const std::vector< D3D11_INPUT_ELEMENT_DESC >& getD3D11InputElements() const { return m_d3dInputElements; }

	uint32_t getD3D11InputElementsHash() const { return m_d3dInputElementsHash; }

protected:
	ComRef< ID3D11Buffer > m_d3dBuffer;
	UINT m_d3dStride;
	UINT m_d3dBaseVertexOffset;
	std::vector< D3D11_INPUT_ELEMENT_DESC > m_d3dInputElements;
	uint32_t m_d3dInputElementsHash;

	VertexBufferDx11(uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_VertexBufferDx11_H
