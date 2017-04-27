/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Dx9/Win32/RenderSystemWin32.h"
#include "Render/Dx9/Win32/RenderViewEmbeddedWin32.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewEmbeddedWin32", RenderViewEmbeddedWin32, RenderViewWin32)

RenderViewEmbeddedWin32::RenderViewEmbeddedWin32(
	RenderSystemWin32* renderSystem,
	ClearTarget* clearTarget,
	ParameterCache* parameterCache,
	IDirect3DDevice9* d3dDevice,
	const D3DPRESENT_PARAMETERS& d3dPresent,
	D3DFORMAT d3dDepthStencilFormat
)
:	RenderViewWin32(renderSystem, clearTarget, parameterCache, d3dDevice)
,	m_d3dPresent(d3dPresent)
,	m_d3dDepthStencilFormat(d3dDepthStencilFormat)
{
}

RenderViewEmbeddedWin32::~RenderViewEmbeddedWin32()
{
	close();
}

bool RenderViewEmbeddedWin32::nextEvent(RenderEvent& outEvent)
{
	return false;
}

bool RenderViewEmbeddedWin32::reset(const RenderViewDefaultDesc& desc)
{
	return false;
}

bool RenderViewEmbeddedWin32::reset(int32_t width, int32_t height)
{
	T_ASSERT (m_renderStateStack.empty());

	HRESULT hr;

	if (!width || !height)
		return false;
	if (m_d3dPresent.BackBufferWidth == width && m_d3dPresent.BackBufferHeight == height)
		return true;

	m_d3dPresent.BackBufferWidth = width;
	m_d3dPresent.BackBufferHeight = height;

	hr = resetDevice();
	return SUCCEEDED(hr);
}

int RenderViewEmbeddedWin32::getWidth() const
{
	return m_d3dPresent.BackBufferWidth;
}

int RenderViewEmbeddedWin32::getHeight() const
{
	return m_d3dPresent.BackBufferHeight;
}

bool RenderViewEmbeddedWin32::isActive() const
{
	if (m_d3dDevice)
		return GetForegroundWindow() == m_d3dPresent.hDeviceWindow;
	else
		return false;
}

bool RenderViewEmbeddedWin32::isMinimized() const
{
	return false;
}

bool RenderViewEmbeddedWin32::isFullScreen() const
{
	return !m_d3dPresent.Windowed;
}

void RenderViewEmbeddedWin32::showCursor()
{
}

void RenderViewEmbeddedWin32::hideCursor()
{
}

bool RenderViewEmbeddedWin32::isCursorVisible() const
{
	return true;
}

SystemWindow RenderViewEmbeddedWin32::getSystemWindow()
{
	SystemWindow sw;
	sw.hWnd = m_d3dPresent.hDeviceWindow;
	return sw;
}

HRESULT RenderViewEmbeddedWin32::lostDevice()
{
	m_d3dSwapChain.release();
	m_d3dBackBuffer.release();
	m_d3dDepthStencilSurface.release();

	for (uint32_t i = 0; i < sizeof_array(m_d3dSyncQueries); ++i)
		m_d3dSyncQueries[i].release();

	m_renderStateStack.clear();
	return S_OK;
}

HRESULT RenderViewEmbeddedWin32::resetDevice()
{
	HRESULT hr;

	hr = m_d3dDevice->CreateAdditionalSwapChain(
		&m_d3dPresent,
		&m_d3dSwapChain.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create additional swap chain; hr = " << hr << Endl;
		return hr;
	}

	hr = m_d3dSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &m_d3dBackBuffer.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to get back buffer; hr = " << hr << Endl;
		return hr;
	}

	hr = m_d3dDevice->CreateDepthStencilSurface(
		m_d3dPresent.BackBufferWidth,
		m_d3dPresent.BackBufferHeight,
		m_d3dDepthStencilFormat,
		m_d3dPresent.MultiSampleType,
		0,
		TRUE,
		&m_d3dDepthStencilSurface.getAssign(),
		NULL
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to get depth/stencil surface; hr = " << hr << Endl;
		return hr;
	}

	for (uint32_t i = 0; i < sizeof_array(m_d3dSyncQueries); ++i)
	{
		hr = m_d3dDevice->CreateQuery(
			D3DQUERYTYPE_EVENT,
			&m_d3dSyncQueries[i].getAssign()
		);
		if (FAILED(hr))
		{
			log::warning << L"Unable to create synchronization query; hr = " << hr << Endl;
			m_d3dSyncQueries[i].release();
		}
	}

	m_d3dViewport.X = 0;
	m_d3dViewport.Y = 0;
	m_d3dViewport.Width = m_d3dPresent.BackBufferWidth;
	m_d3dViewport.Height = m_d3dPresent.BackBufferHeight;
	m_d3dViewport.MinZ = 0.0f;
	m_d3dViewport.MaxZ = 1.0f;

	return S_OK;
}

	}
}
