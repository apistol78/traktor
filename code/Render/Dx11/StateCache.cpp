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
,	m_d3dTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
,	m_d3dSignatureHash(0)
,	m_stencilReference(0)
{
}

void StateCache::reset()
{
	m_d3dRasterizerState = 0;
	m_d3dDepthStencilState = 0;
	m_d3dBlendState = 0;
	m_d3dVertexShader = 0;
	m_d3dPixelShader = 0;
	m_d3dVertexBuffer = 0;
	m_d3dIndexBuffer = 0;
	m_d3dTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	m_d3dSignatureHash = 0;
	m_stencilReference = 0;
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

void StateCache::setVertexBuffer(ID3D11Buffer* d3dVertexBuffer, UINT d3dVertexStride)
{
	if (d3dVertexBuffer != m_d3dVertexBuffer)
	{
		UINT offset = 0;
		m_d3dDeviceContext->IASetVertexBuffers(0, 1, &d3dVertexBuffer, &d3dVertexStride, &offset);
		m_d3dVertexBuffer = d3dVertexBuffer;
	}
}

void StateCache::setIndexBuffer(ID3D11Buffer* d3dIndexBuffer, DXGI_FORMAT d3dIndexFormat)
{
	if (d3dIndexBuffer != m_d3dIndexBuffer)
	{
		m_d3dDeviceContext->IASetIndexBuffer(d3dIndexBuffer, d3dIndexFormat, 0);
		m_d3dIndexBuffer = d3dIndexBuffer;
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

void StateCache::setInputLayout(uint32_t d3dVertexShaderHash, ID3DBlob* d3dVertexShaderBlob, uint32_t d3dInputElementsHash, const std::vector< D3D11_INPUT_ELEMENT_DESC >& d3dInputElements)
{
	uint64_t d3dSignatureHash = (uint64_t(d3dVertexShaderHash) << 32) | d3dInputElementsHash;
	if (m_d3dSignatureHash != d3dSignatureHash)
	{
		ID3D11InputLayout* d3dInputLayout = 0;
		SmallMap< uint64_t, ComRef< ID3D11InputLayout > >::iterator i = m_d3dInputLayouts.find(d3dSignatureHash);
		if (i != m_d3dInputLayouts.end())
			d3dInputLayout = i->second;
		else
		{
			HRESULT hr;

			// Get device from context.
			ComRef< ID3D11Device > d3dDevice;
			m_d3dDeviceContext->GetDevice(&d3dDevice.getAssign());
			T_ASSERT (d3dDevice);

			// Layout hasn't been mapped yet; create new input layout.
			hr = d3dDevice->CreateInputLayout(
				&d3dInputElements[0],
				UINT(d3dInputElements.size()),
				d3dVertexShaderBlob->GetBufferPointer(),
				d3dVertexShaderBlob->GetBufferSize(),
				&d3dInputLayout
			);
			T_ASSERT (SUCCEEDED(hr));
			T_ASSERT (d3dInputLayout);

			// Save layout for later binding.
			m_d3dInputLayouts[d3dSignatureHash] = d3dInputLayout;
		}

		m_d3dSignatureHash = d3dSignatureHash;
		m_d3dDeviceContext->IASetInputLayout(d3dInputLayout);
	}
}

	}
}
