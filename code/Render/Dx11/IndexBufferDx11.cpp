#include "Render/Dx11/IndexBufferDx11.h"
#include "Render/Dx11/StateCache.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferDx11", IndexBufferDx11, IndexBuffer)

void IndexBufferDx11::prepare(ID3D11DeviceContext* d3dDeviceContext, StateCache& stateCache)
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

	stateCache.setIndexBuffer(m_d3dBuffer, indexFormat);
}

IndexBufferDx11::IndexBufferDx11(IndexType indexType, uint32_t bufferSize)
:	IndexBuffer(indexType, bufferSize)
,	m_d3dBaseIndexOffset(0)
{
}

	}
}
