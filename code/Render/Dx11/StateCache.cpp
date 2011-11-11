#include "Core/Misc/Adler32.h"
#include "Render/Dx11/StateCache.h"

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

StateCache::StateCache(ID3D11Device* d3dDevice)
:	m_d3dDevice(d3dDevice)
{
}

ID3D11RasterizerState* StateCache::getRasterizerState(const D3D11_RASTERIZER_DESC& rd)
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

ID3D11DepthStencilState* StateCache::getDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& dsd)
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

ID3D11BlendState* StateCache::getBlendState(const D3D11_BLEND_DESC& bd)
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

	}
}
