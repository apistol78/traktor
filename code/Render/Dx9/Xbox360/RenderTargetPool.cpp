/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Render/Dx9/Xbox360/RenderTargetPool.h"
#include "Core/Log/Log.h"
#undef max

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetPool", RenderTargetPool, Object)

RenderTargetPool::RenderTargetPool()
:	m_edramBase(0)
{
	for (int i = 0; i < 10; ++i)
	{
		Target* target = new Target();

		target->d3dFormat = D3DFMT_UNKNOWN;
		target->d3dMultisample = D3DMULTISAMPLE_NONE;
		target->width = 0;
		target->height = 0;
		target->d3dSurface = 0;

		m_targetPool.push_back(target);
	}
}

void RenderTargetPool::destroy(IDirect3DDevice9* d3dDevice)
{
}

RenderTargetPool::Target* RenderTargetPool::acquire(
	IDirect3DDevice9* d3dDevice,
	int width,
	int height,
	D3DFORMAT d3dFormat,
	D3DMULTISAMPLE_TYPE d3dMultisample
)
{
	T_ASSERT_M(!m_targetPool.empty(), L"Too many targets acquired");

	Target* target = m_targetPool.back();

	HRESULT hr = d3dDevice->CreateRenderTarget(
		width,
		height,
		d3dFormat,
		d3dMultisample,
		0,
		0,
		&target->d3dSurface,
		NULL
	);
	if (FAILED(hr))
		return 0;

	target->d3dFormat = d3dFormat;
	target->d3dMultisample = d3dMultisample;
	target->width = width;
	target->height = height;

	m_targetPool.pop_back();
	return target;
}

void RenderTargetPool::release(Target*& target)
{
	target->d3dSurface->Release();
	target->d3dSurface = 0;

	m_targetPool.push_back(target);
	target = 0;
}

	}
}
