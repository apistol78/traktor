#include <xmmintrin.h>
#include "Core/Memory/Alloc.h"
#include "Render/Dx9/ParameterCache.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

bool compareExchangeEqual4(float* ptr1, const float* ptr2, size_t count)
{
	T_ASSERT ((count & 3) == 0);

	bool equal = true;

	for (size_t i = 0; i < (count >> 2); ++i)
	{
		__m128 r1 = _mm_load_ps(ptr1);
		__m128 r2 = _mm_load_ps(ptr2);
		__m128 eq =_mm_cmpeq_ps(r1, r2);

		if (_mm_movemask_ps(eq) != 15)
		{
			_mm_store_ps(ptr1, r2);
			equal = false;
		}

		ptr1 += 4;
		ptr2 += 4;
	}

	return equal;
}

		}

ParameterCache::ParameterCache(UnmanagedListener* listener, IDirect3DDevice9* d3dDevice)
:	Unmanaged(listener)
,	m_d3dVertexShader(0)
,	m_d3dPixelShader(0)
,	m_vertexConstantsShadow(0)
,	m_pixelConstantsShadow(0)
{
	Unmanaged::addToListener();

	m_vertexConstantsShadow = (float*)Alloc::acquireAlign(VertexConstantCount * 4 * sizeof(float), 16);
	m_pixelConstantsShadow = (float*)Alloc::acquireAlign(PixelConstantCount * 4 * sizeof(float), 16);

	lostDevice();
	resetDevice(d3dDevice);
}

ParameterCache::~ParameterCache()
{
	Unmanaged::removeFromListener();

	Alloc::freeAlign(m_pixelConstantsShadow);
	Alloc::freeAlign(m_vertexConstantsShadow);
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
	float* shadow = &m_vertexConstantsShadow[registerOffset * 4];
	if (!compareExchangeEqual4(shadow, constantData, registerCount * 4))
		m_d3dDevice->SetVertexShaderConstantF(registerOffset, constantData, registerCount);
}

void ParameterCache::setPixelShaderConstant(uint32_t registerOffset, uint32_t registerCount, const float* constantData)
{
	float* shadow = &m_pixelConstantsShadow[registerOffset * 4];
	if (!compareExchangeEqual4(shadow, constantData, registerCount * 4))
		m_d3dDevice->SetPixelShaderConstantF(registerOffset, constantData, registerCount);
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

	std::memset(m_vertexConstantsShadow, 0, VertexConstantCount * 4 * sizeof(float));
	std::memset(m_pixelConstantsShadow, 0, PixelConstantCount * 4 * sizeof(float));

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
