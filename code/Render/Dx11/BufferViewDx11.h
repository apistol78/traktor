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

	explicit BufferViewDx11(ID3D11Buffer* d3dBuffer, ID3D11ShaderResourceView* d3dBufferResourceView);

	ID3D11Buffer* getD3D11Buffer() const { return m_d3dBuffer; }

	ID3D11ShaderResourceView* getD3D11ShaderResourceView() const { return m_d3dBufferResourceView; }

private:
	ID3D11Buffer* m_d3dBuffer = nullptr;
	ID3D11ShaderResourceView* m_d3dBufferResourceView = nullptr;
};

	}
}
