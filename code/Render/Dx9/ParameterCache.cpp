#include "Render/Dx9/ParameterCache.h"

namespace traktor
{
	namespace render
	{

ParameterCache::ParameterCache(UnmanagedListener* listener, IDirect3DDevice9* d3dDevice)
:	Unmanaged(listener)
{
	Unmanaged::addToListener();
	lostDevice();
	resetDevice(d3dDevice);
}

ParameterCache::~ParameterCache()
{
	Unmanaged::removeFromListener();
}

void ParameterCache::setVertexShader(IDirect3DVertexShader9* d3dVertexShader)
{
	if (m_d3dVertexShader != d3dVertexShader)
	{
		m_d3dDevice->SetVertexShader(d3dVertexShader);
		m_d3dVertexShader = d3dVertexShader;
	}
}

void ParameterCache::setPixelShader(IDirect3DPixelShader9* d3dPixelShader)
{
	if (m_d3dPixelShader != d3dPixelShader)
	{
		m_d3dDevice->SetPixelShader(d3dPixelShader);
		m_d3dPixelShader = d3dPixelShader;
	}
}

void ParameterCache::setVertexShaderConstant(uint32_t registerOffset, uint32_t registerCount, const float* constantData)
{
	float* shadow = &m_vertexConstantsShadow[registerOffset];

	if (std::memcmp(shadow, constantData, registerCount * 4 * sizeof(float)) == 0)
		return;

	m_d3dDevice->SetVertexShaderConstantF(registerOffset, constantData, registerCount);

	std::memcpy(shadow, constantData, registerCount * 4 * sizeof(float));
}

void ParameterCache::setPixelShaderConstant(uint32_t registerOffset, uint32_t registerCount, const float* constantData)
{
	float* shadow = &m_pixelConstantsShadow[registerOffset];

	if (std::memcmp(shadow, constantData, registerCount * 4 * sizeof(float)) == 0)
		return;

	m_d3dDevice->SetPixelShaderConstantF(registerOffset, constantData, registerCount);

	std::memcpy(shadow, constantData, registerCount * 4 * sizeof(float));
}

void ParameterCache::setVertexTexture(uint32_t stage, IDirect3DBaseTexture9* d3dTexture)
{
	if (m_vertexTextureShadow[stage] == d3dTexture)
		return;

	m_d3dDevice->SetTexture(D3DVERTEXTEXTURESAMPLER0 + stage, d3dTexture);

	m_vertexTextureShadow[stage] = d3dTexture;
}

void ParameterCache::setPixelTexture(uint32_t stage, IDirect3DBaseTexture9* d3dTexture)
{
	if (m_pixelTextureShadow[stage] == d3dTexture)
		return;

	m_d3dDevice->SetTexture(stage, d3dTexture);

	m_pixelTextureShadow[stage] = d3dTexture;
}

void ParameterCache::setRenderState(uint32_t state, uint32_t value)
{
	if (m_renderStates[state] == value)
		return;

#if !defined(_XBOX)
	m_d3dDevice->SetRenderState((D3DRENDERSTATETYPE)state, value);
#else
	m_d3dDevice->SetRenderState_Inline((D3DRENDERSTATETYPE)state, value);
#endif

	m_renderStates[state] = value;
}

void ParameterCache::setSamplerState(uint32_t sampler, uint32_t state, uint32_t value)
{
	if (m_samplerStates[sampler][state] == value)
		return;

#if !defined(_XBOX)
	m_d3dDevice->SetSamplerState(sampler, (D3DSAMPLERSTATETYPE)state, value);
#else
	m_d3dDevice->SetSamplerState_Inline(sampler, (D3DSAMPLERSTATETYPE)state, value);
#endif

	m_samplerStates[sampler][state] = value;
}

HRESULT ParameterCache::lostDevice()
{
	m_d3dDevice = 0;
	m_d3dVertexShader = 0;
	m_d3dPixelShader = 0;

	std::memset(m_vertexConstantsShadow, 0, sizeof(m_vertexConstantsShadow));
	std::memset(m_pixelConstantsShadow, 0, sizeof(m_pixelConstantsShadow));

	for (int i = 0; i < sizeof_array(m_vertexTextureShadow); ++i)
		m_vertexTextureShadow[i] = 0;

	for (int i = 0; i < sizeof_array(m_pixelTextureShadow); ++i)
		m_pixelTextureShadow[i] = 0;

	m_renderStates = std::vector< uint32_t >(256, ~0UL);
	for (int i = 0; i < sizeof_array(m_samplerStates); ++i)
		m_samplerStates[i] = std::vector< uint32_t >(16, ~0UL);

	return S_OK;
}

HRESULT ParameterCache::resetDevice(IDirect3DDevice9* d3dDevice)
{
	m_d3dDevice = d3dDevice;
	return S_OK;
}

	}
}
