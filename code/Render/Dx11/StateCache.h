#ifndef traktor_render_StateCache_H
#define traktor_render_StateCache_H

#include "Core/Object.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

class StateCache : public Object
{
public:
	StateCache(ID3D11DeviceContext* d3dDeviceContext);

	void setRasterizerState(ID3D11RasterizerState* d3dRasterizerState);

	void setDepthStencilState(ID3D11DepthStencilState* d3dDepthStencilState, UINT stencilReference);

	void setBlendState(ID3D11BlendState* d3dBlendState);

	void setVertexShader(ID3D11VertexShader* d3dVertexShader);

	void setPixelShader(ID3D11PixelShader* d3dPixelShader);

	void setTopology(D3D11_PRIMITIVE_TOPOLOGY d3dTopology);

private:
	ComRef< ID3D11DeviceContext > m_d3dDeviceContext;
	ComRef< ID3D11RasterizerState > m_d3dRasterizerState;
	ComRef< ID3D11DepthStencilState > m_d3dDepthStencilState;
	ComRef< ID3D11BlendState > m_d3dBlendState;
	ComRef< ID3D11VertexShader > m_d3dVertexShader;
	ComRef< ID3D11PixelShader > m_d3dPixelShader;
	D3D11_PRIMITIVE_TOPOLOGY m_d3dTopology;
	UINT m_stencilReference;
};

	}
}

#endif	// traktor_render_StateCache_H
