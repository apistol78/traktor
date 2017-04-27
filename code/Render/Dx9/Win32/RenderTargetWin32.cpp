/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Render/Types.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Dx9/VertexBufferDx9.h"
#include "Render/Dx9/Win32/RenderTargetWin32.h"
#include "Render/Dx9/Win32/ProgramWin32.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetWin32", RenderTargetWin32, ISimpleTexture)

RenderTargetWin32::RenderTargetWin32()
:	m_width(0)
,	m_height(0)
,	m_d3dFormat(D3DFMT_UNKNOWN)
{
}

bool RenderTargetWin32::create(
	IDirect3DDevice9* d3dDevice,
	const RenderTargetSetCreateDesc& setDesc,
	const RenderTargetCreateDesc& targetDesc,
	D3DFORMAT d3dFormat
)
{
	HRESULT hr;

	D3DMULTISAMPLE_TYPE d3dMultiSample = c_d3dMultiSample[setDesc.multiSample];
	if (d3dMultiSample == D3DMULTISAMPLE_NONE)
	{
		hr = d3dDevice->CreateTexture(
			setDesc.width,
			setDesc.height,
			1,
			D3DUSAGE_RENDERTARGET,
			d3dFormat,
			D3DPOOL_DEFAULT,
			&m_d3dTargetTexture.getAssign(),
			NULL
		);
		if (FAILED(hr))
		{
			log::error << L"Render target create failed; Unable to create render target texture" << Endl;
			return false;
		}

		hr = m_d3dTargetTexture->GetSurfaceLevel(0, &m_d3dTargetSurface.getAssign());
		if (FAILED(hr))
		{
			log::error << L"Render target create failed; Unable to get color surface level 0" << Endl;
			return false;
		}
	}
	else
	{
		hr = d3dDevice->CreateRenderTarget(
			setDesc.width,
			setDesc.height,
			d3dFormat,
			d3dMultiSample,
			0,
			FALSE,
			&m_d3dTargetSurface.getAssign(),
			NULL
		);
		if (FAILED(hr))
		{
			log::error << L"Render target create failed; Unable to create render target" << Endl;
			return false;
		}

		hr = d3dDevice->CreateTexture(
			setDesc.width,
			setDesc.height,
			1,
			D3DUSAGE_RENDERTARGET,
			d3dFormat,
			D3DPOOL_DEFAULT,
			&m_d3dTargetTexture.getAssign(),
			NULL
		);
		if (FAILED(hr))
		{
			log::error << L"Render target create failed; Unable to create render target resolve texture" << Endl;
			return false;
		}

		hr = m_d3dTargetTexture->GetSurfaceLevel(0, &m_d3dResolveTargetSurface.getAssign());
		if (FAILED(hr))
		{
			log::error << L"Render target create failed; Unable to get resolve color surface level 0" << Endl;
			return false;
		}
	}

	m_width = setDesc.width;
	m_height = setDesc.height;
	m_d3dFormat = d3dFormat;

	return true;
}

void RenderTargetWin32::release()
{
	m_d3dTargetTexture.release();
	m_d3dTargetSurface.release();
	m_d3dResolveTargetSurface.release();
}

bool RenderTargetWin32::resolve(IDirect3DDevice9* d3dDevice)
{
	if (!m_d3dResolveTargetSurface)
		return true;

	HRESULT hr = d3dDevice->StretchRect(
		m_d3dTargetSurface,
		NULL,
		m_d3dResolveTargetSurface,
		NULL,
		D3DTEXF_NONE
	);

	return SUCCEEDED(hr);
}

void RenderTargetWin32::destroy()
{
	T_FATAL_ERROR;
}

ITexture* RenderTargetWin32::resolve()
{
	return this;
}

int RenderTargetWin32::getWidth() const
{
	return m_width;
}

int RenderTargetWin32::getHeight() const
{
	return m_height;
}

bool RenderTargetWin32::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetWin32::unlock(int level)
{
}

void* RenderTargetWin32::getInternalHandle()
{
	return m_d3dTargetTexture;
}

	}
}
