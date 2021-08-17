#pragma once

#include "Render/IBufferView.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

class BufferViewDx11 : public IBufferView
{
	T_RTTI_CLASS;

public:
	BufferViewDx11() = default;

	explicit BufferViewDx11(ID3D11ShaderResourceView* d3dBufferResourceView);

	ID3D11ShaderResourceView* getD3D11ShaderResourceView() const { return m_d3dBufferResourceView; }

private:
	ID3D11ShaderResourceView* m_d3dBufferResourceView = nullptr;
};

	}
}
