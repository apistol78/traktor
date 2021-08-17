#include "Render/Dx11/BufferViewDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferViewDx11", BufferViewDx11, IBufferView)

BufferViewDx11::BufferViewDx11(ID3D11ShaderResourceView* d3dBufferResourceView)
:	m_d3dBufferResourceView(d3dBufferResourceView)
{
}

	}
}