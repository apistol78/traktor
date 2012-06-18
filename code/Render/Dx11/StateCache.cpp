#include "Render/Dx11/StateCache.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const float c_blendFactors[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		}

StateCache::StateCache(ID3D11DeviceContext* d3dDeviceContext)
:	m_d3dDeviceContext(d3dDeviceContext)
,	m_stencilReference(0)
,	m_d3dTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
{
}

void StateCache::setRasterizerState(ID3D11RasterizerState* d3dRasterizerState)
{
	if (d3dRasterizerState != m_d3dRasterizerState)
	{
		m_d3dDeviceContext->RSSetState(d3dRasterizerState);
		m_d3dRasterizerState = d3dRasterizerState;
	}
}

void StateCache::setDepthStencilState(ID3D11DepthStencilState* d3dDepthStencilState, UINT stencilReference)
{
	if (d3dDepthStencilState != m_d3dDepthStencilState || stencilReference != m_stencilReference)
	{
		m_d3dDeviceContext->OMSetDepthStencilState(d3dDepthStencilState, stencilReference);
		m_d3dDepthStencilState = d3dDepthStencilState;
		m_stencilReference = stencilReference;
	}
}

void StateCache::setBlendState(ID3D11BlendState* d3dBlendState)
{
	if (d3dBlendState != m_d3dBlendState)
	{
		m_d3dDeviceContext->OMSetBlendState(d3dBlendState, c_blendFactors, 0xffffffff);
		m_d3dBlendState = d3dBlendState;
	}
}

void StateCache::setVertexShader(ID3D11VertexShader* d3dVertexShader)
{
	if (d3dVertexShader != m_d3dVertexShader)
	{
		m_d3dDeviceContext->VSSetShader(d3dVertexShader, 0, 0);
		m_d3dVertexShader = d3dVertexShader;
	}
}

void StateCache::setPixelShader(ID3D11PixelShader* d3dPixelShader)
{
	if (d3dPixelShader != m_d3dPixelShader)
	{
		m_d3dDeviceContext->PSSetShader(d3dPixelShader, 0, 0);
		m_d3dPixelShader = d3dPixelShader;
	}
}

void StateCache::setTopology(D3D11_PRIMITIVE_TOPOLOGY d3dTopology)
{
	if (d3dTopology != m_d3dTopology)
	{
		m_d3dDeviceContext->IASetPrimitiveTopology(d3dTopology);
		m_d3dTopology = d3dTopology;
	}
}

	}
}
