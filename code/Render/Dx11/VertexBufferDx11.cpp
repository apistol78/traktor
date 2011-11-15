#include "Render/Dx11/VertexBufferDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDx11", VertexBufferDx11, VertexBuffer)

void VertexBufferDx11::prepare(ID3D11DeviceContext* d3dDeviceContext)
{
	ID3D11Buffer* d3dBuffer = m_d3dBuffer;
	UINT stride = m_d3dStride, offset = 0;
	d3dDeviceContext->IASetVertexBuffers(0, 1, &d3dBuffer, &stride, &offset);
}

VertexBufferDx11::VertexBufferDx11(uint32_t bufferSize)
:	VertexBuffer(bufferSize)
{
}

	}
}
