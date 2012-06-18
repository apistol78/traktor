#ifndef traktor_render_ResourceCache_H
#define traktor_render_ResourceCache_H

#include <map>
#include "Core/Object.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx11/Platform.h"

namespace traktor
{
	namespace render
	{

class ResourceCache : public Object
{
public:
	ResourceCache(ID3D11Device* d3dDevice);

	ID3D11RasterizerState* getRasterizerState(const D3D11_RASTERIZER_DESC& rd);

	ID3D11DepthStencilState* getDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& dsd);

	ID3D11BlendState* getBlendState(const D3D11_BLEND_DESC& bd);

	ID3D11VertexShader* getVertexShader(ID3DBlob* vertexShaderBlob, uint32_t vertexShaderHash);

	ID3D11PixelShader* getPixelShader(ID3DBlob* pixelShaderBlob, uint32_t pixelShaderHash);

private:
	ComRef< ID3D11Device > m_d3dDevice;
	std::map< uint32_t, ComRef< ID3D11RasterizerState > > m_d3dRasterizerStates;
	std::map< uint32_t, ComRef< ID3D11DepthStencilState > > m_d3dDepthStencilStates;
	std::map< uint32_t, ComRef< ID3D11BlendState > > m_d3dBlendStates;
	std::map< uint32_t, ComRef< ID3D11VertexShader > > m_d3dVertexShaders;
	std::map< uint32_t, ComRef< ID3D11PixelShader > > m_d3dPixelShaders;
};

	}
}

#endif	// traktor_render_ResourceCache_H
