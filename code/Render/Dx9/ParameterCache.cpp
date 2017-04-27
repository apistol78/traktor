/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if !defined(_XBOX)
#	include <xmmintrin.h>
#endif
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
#if !defined(_XBOX)
	T_ASSERT ((count & 3) == 0);

	const float* src = ptr2;
	float* dst = ptr1;
	bool equal = true;

	for (size_t i = 0; i < (count >> 2); ++i)
	{
		__m128 r1 = _mm_load_ps(dst);
		__m128 r2 = _mm_load_ps(src);
		__m128 eq =_mm_cmpeq_ps(r1, r2);

		int mask = _mm_movemask_ps(eq);
		if (mask != 15)
		{
			_mm_store_ps(dst, r2);
			equal = false;
		}

		src += 4;
		dst += 4;
	}

	return equal;
#else
	return false;
#endif
}

		}

ParameterCache::ParameterCache(IDirect3DDevice9* d3dDevice, float mipBias, DWORD maxAnisotropy)
:	m_d3dVertexShader(0)
,	m_d3dPixelShader(0)
,	m_vertexConstantsShadow(0)
,	m_pixelConstantsShadow(0)
,	m_mipBias(mipBias)
,	m_maxAnisotropy(maxAnisotropy)
{
	m_vertexConstantsShadow.reset((float*)Alloc::acquireAlign(VertexConstantCount * 4 * sizeof(float), 16, T_FILE_LINE));
	m_pixelConstantsShadow.reset((float*)Alloc::acquireAlign(PixelConstantCount * 4 * sizeof(float), 16, T_FILE_LINE));

	lostDevice();
	resetDevice(d3dDevice);
}

ParameterCache::~ParameterCache()
{
	m_pixelConstantsShadow.release();
	m_vertexConstantsShadow.release();
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
	float* shadow = m_vertexConstantsShadow.ptr() + registerOffset * 4;
	if (!compareExchangeEqual4(shadow, constantData, registerCount * 4))
		m_d3dDevice->SetVertexShaderConstantF(registerOffset, constantData, registerCount);
#if defined(_DEBUG)
	else if (registerCount <= 1)
	{
		float deviceConstantData[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_d3dDevice->GetVertexShaderConstantF(registerOffset, deviceConstantData, registerCount);
		T_ASSERT (
			deviceConstantData[0] == constantData[0] &&
			deviceConstantData[1] == constantData[1] &&
			deviceConstantData[2] == constantData[2] &&
			deviceConstantData[3] == constantData[3]
		);
	}
#endif
}

void ParameterCache::setPixelShaderConstant(uint32_t registerOffset, uint32_t registerCount, const float* constantData)
{
	float* shadow = m_pixelConstantsShadow.ptr() + registerOffset * 4;
	if (!compareExchangeEqual4(shadow, constantData, registerCount * 4))
		m_d3dDevice->SetPixelShaderConstantF(registerOffset, constantData, registerCount);
#if defined(_DEBUG)
	else if (registerCount <= 1)
	{
		float deviceConstantData[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_d3dDevice->GetPixelShaderConstantF(registerOffset, deviceConstantData, registerCount);
		T_ASSERT (
			deviceConstantData[0] == constantData[0] &&
			deviceConstantData[1] == constantData[1] &&
			deviceConstantData[2] == constantData[2] &&
			deviceConstantData[3] == constantData[3]
		);
	}
#endif
}

void ParameterCache::setVertexShaderConstantAlways(uint32_t registerOffset, uint32_t registerCount, const float* constantData)
{
	float* shadow = m_vertexConstantsShadow.ptr() + registerOffset * 4;
	m_d3dDevice->SetVertexShaderConstantF(registerOffset, constantData, registerCount);
	std::memcpy(shadow, constantData, registerCount * 4 * sizeof(float));
}

void ParameterCache::setPixelShaderConstantAlways(uint32_t registerOffset, uint32_t registerCount, const float* constantData)
{
	float* shadow = m_pixelConstantsShadow.ptr() + registerOffset * 4;
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

void ParameterCache::setVertexSamplerState(uint32_t sampler, uint32_t state, uint32_t value)
{
	T_ASSERT (sampler < MaxVertexSamplerCount);
	T_ASSERT (state < m_vertexSamplerStates[sampler].size());

	if (m_vertexSamplerStates[sampler][state] == value)
		return;

#if !defined(_XBOX)
	m_d3dDevice->SetSamplerState(sampler + D3DVERTEXTEXTURESAMPLER0, (D3DSAMPLERSTATETYPE)state, value);
#else
	m_d3dDevice->SetSamplerState_Inline(sampler + D3DVERTEXTEXTURESAMPLER0, (D3DSAMPLERSTATETYPE)state, value);
#endif

	m_vertexSamplerStates[sampler][state] = value;
}

void ParameterCache::setPixelSamplerState(uint32_t sampler, uint32_t state, uint32_t value)
{
	T_ASSERT (sampler < MaxPixelSamplerCount);
	T_ASSERT (state < m_pixelSamplerStates[sampler].size());

	if (m_pixelSamplerStates[sampler][state] == value)
		return;

#if !defined(_XBOX)
	m_d3dDevice->SetSamplerState(sampler, (D3DSAMPLERSTATETYPE)state, value);
#else
	m_d3dDevice->SetSamplerState_Inline(sampler, (D3DSAMPLERSTATETYPE)state, value);
#endif

	m_pixelSamplerStates[sampler][state] = value;
}

HRESULT ParameterCache::lostDevice()
{
	m_d3dDevice = 0;
	m_d3dVertexShader = 0;
	m_d3dPixelShader = 0;

	std::memset(m_vertexConstantsShadow.ptr(), 0, VertexConstantCount * 4 * sizeof(float));
	std::memset(m_pixelConstantsShadow.ptr(), 0, PixelConstantCount * 4 * sizeof(float));

	for (int i = 0; i < sizeof_array(m_vertexTextureShadow); ++i)
		m_vertexTextureShadow[i] = 0;

	for (int i = 0; i < sizeof_array(m_pixelTextureShadow); ++i)
		m_pixelTextureShadow[i] = 0;

	m_renderStates = std::vector< uint32_t >(256, ~0UL);
	for (int i = 0; i < sizeof_array(m_vertexSamplerStates); ++i)
		m_vertexSamplerStates[i] = std::vector< uint32_t >(16, ~0UL);
	for (int i = 0; i < sizeof_array(m_pixelSamplerStates); ++i)
		m_pixelSamplerStates[i] = std::vector< uint32_t >(16, ~0UL);

	return S_OK;
}

HRESULT ParameterCache::resetDevice(IDirect3DDevice9* d3dDevice)
{
	m_d3dDevice = d3dDevice;

	m_d3dDevice->SetVertexShaderConstantF(0, m_vertexConstantsShadow.ptr(), VertexConstantCount);
	m_d3dDevice->SetPixelShaderConstantF(0, m_pixelConstantsShadow.ptr(), PixelConstantCount);

	for (int i = 0; i < MaxPixelSamplerCount; ++i)
	{
		m_d3dDevice->SetSamplerState(i, D3DSAMP_MIPMAPLODBIAS, *(DWORD*)&m_mipBias);
		m_d3dDevice->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, m_maxAnisotropy);
	}

	return S_OK;
}

	}
}
