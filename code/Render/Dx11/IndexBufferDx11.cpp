#include "Render/Dx11/IndexBufferDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferDx11", IndexBufferDx11, IndexBuffer)

void IndexBufferDx11::prepare(ID3D11DeviceContext* d3dDeviceContext)
{
	DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN;

	switch (getIndexType())
	{
	case ItUInt16:
		indexFormat = DXGI_FORMAT_R16_UINT;
		break;

	case ItUInt32:
		indexFormat = DXGI_FORMAT_R32_UINT;
		break;
	}

	d3dDeviceContext->IASetIndexBuffer(m_d3dBuffer, indexFormat, 0);
}

IndexBufferDx11::IndexBufferDx11(IndexType indexType, uint32_t bufferSize)
:	IndexBuffer(indexType, bufferSize)
{
}

	}
}
