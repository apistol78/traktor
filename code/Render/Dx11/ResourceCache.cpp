#include "Core/Misc/Adler32.h"
#include "Render/Dx11/ResourceCache.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

template < typename T >
uint32_t hash(const T& item)
{
	Adler32 a;
	a.begin();
	a.feed(&item, sizeof(T));
	a.end();
	return a.get();
}

		}

ResourceCache::ResourceCache(ID3D11Device* d3dDevice)
:	m_d3dDevice(d3dDevice)
{
}

ID3D11RasterizerState* ResourceCache::getRasterizerState(const D3D11_RASTERIZER_DESC& rd)
{
	uint32_t h = hash(rd);

	std::map< uint32_t, ComRef< ID3D11RasterizerState > >::iterator i = m_d3dRasterizerStates.find(h);
	if (i != m_d3dRasterizerStates.end())
		return i->second;
	
	ComRef< ID3D11RasterizerState > rs;

	HRESULT hr = m_d3dDevice->CreateRasterizerState(
		&rd,
		&rs.getAssign()
	);
	if (FAILED(hr))
		return 0;

	m_d3dRasterizerStates.insert(std::make_pair(h, rs));
	return rs;
}

ID3D11DepthStencilState* ResourceCache::getDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& dsd)
{
	uint32_t h = hash(dsd);

	std::map< uint32_t, ComRef< ID3D11DepthStencilState > >::iterator i = m_d3dDepthStencilStates.find(h);
	if (i != m_d3dDepthStencilStates.end())
		return i->second;
	
	ComRef< ID3D11DepthStencilState > dss;

	HRESULT hr = m_d3dDevice->CreateDepthStencilState(
		&dsd,
		&dss.getAssign()
	);
	if (FAILED(hr))
		return 0;

	m_d3dDepthStencilStates.insert(std::make_pair(h, dss));
	return dss;
}

ID3D11BlendState* ResourceCache::getBlendState(const D3D11_BLEND_DESC& bd)
{
	uint32_t h = hash(bd);

	std::map< uint32_t, ComRef< ID3D11BlendState > >::iterator i = m_d3dBlendStates.find(h);
	if (i != m_d3dBlendStates.end())
		return i->second;
	
	ComRef< ID3D11BlendState > bs;

	HRESULT hr = m_d3dDevice->CreateBlendState(
		&bd,
		&bs.getAssign()
	);
	if (FAILED(hr))
		return 0;

	m_d3dBlendStates.insert(std::make_pair(h, bs));
	return bs;
}

ID3D11VertexShader* ResourceCache::getVertexShader(ID3DBlob* vertexShaderBlob, uint32_t vertexShaderHash)
{
	ComRef< ID3D11VertexShader > d3dVertexShader;
	HRESULT hr;

	std::map< uint32_t, ComRef< ID3D11VertexShader > >::const_iterator i = m_d3dVertexShaders.find(vertexShaderHash);
	if (i != m_d3dVertexShaders.end())
		return i->second;

	hr = m_d3dDevice->CreateVertexShader(
		(DWORD*)vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize(),
		0,
		&d3dVertexShader.getAssign()
	);
	if (FAILED(hr))
		return 0;
	
	m_d3dVertexShaders.insert(std::make_pair(vertexShaderHash, d3dVertexShader));
	return d3dVertexShader;
}

ID3D11PixelShader* ResourceCache::getPixelShader(ID3DBlob* pixelShaderBlob, uint32_t pixelShaderHash)
{
	ComRef< ID3D11PixelShader > d3dPixelShader;
	HRESULT hr;

	std::map< uint32_t, ComRef< ID3D11PixelShader > >::const_iterator i = m_d3dPixelShaders.find(pixelShaderHash);
	if (i != m_d3dPixelShaders.end())
		return i->second;

	hr = m_d3dDevice->CreatePixelShader(
		(DWORD*)pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize(),
		0,
		&d3dPixelShader.getAssign()
	);
	if (FAILED(hr))
		return 0;
	
	m_d3dPixelShaders.insert(std::make_pair(pixelShaderHash, d3dPixelShader));
	return d3dPixelShader;
}

	}
}
