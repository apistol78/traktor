#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/Dx9/IndexBufferDx9.h"
#include "Render/Dx9/ParameterCache.h"
#include "Render/Dx9/VertexBufferDx9.h"
#include "Render/Dx9/Win32/ProgramWin32.h"
#include "Render/Dx9/Win32/RenderViewWin32.h"
#include "Render/Dx9/Win32/RenderSystemWin32.h"
#include "Render/Dx9/Win32/RenderTargetSetWin32.h"
#include "Render/Dx9/Win32/RenderTargetWin32.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const DWORD c_d3dClearMask[] =
{
	0,
	D3DCLEAR_TARGET,
	D3DCLEAR_ZBUFFER,
	D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
	D3DCLEAR_STENCIL,
	D3DCLEAR_TARGET | D3DCLEAR_STENCIL,
	D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
	D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL
};

const D3DPRIMITIVETYPE c_d3dPrimitiveType[] =
{ 
	D3DPT_POINTLIST,
	D3DPT_LINESTRIP,
	D3DPT_LINELIST,
	D3DPT_TRIANGLESTRIP,
	D3DPT_TRIANGLELIST
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewWin32", RenderViewWin32, IRenderView)

RenderViewWin32::RenderViewWin32(
	RenderSystemWin32* renderSystem,
	ParameterCache* parameterCache,
	const RenderViewCreateDesc& createDesc,
	const D3DPRESENT_PARAMETERS& d3dPresent,
	D3DFORMAT d3dDepthStencilFormat,
	float nativeAspectRatio
)
:	m_renderSystem(renderSystem)
,	m_parameterCache(parameterCache)
,	m_createDesc(createDesc)
,	m_d3dDevice(0)
,	m_d3dPresent(d3dPresent)
,	m_d3dDepthStencilFormat(d3dDepthStencilFormat)
,	m_nativeAspectRatio(nativeAspectRatio)
,	m_frameCount(0)
,	m_targetDirty(false)
{
	m_d3dViewport.X = 0;
	m_d3dViewport.Y = 0;
	m_d3dViewport.Width = m_d3dPresent.BackBufferWidth;
	m_d3dViewport.Height = m_d3dPresent.BackBufferHeight;
	m_d3dViewport.MinZ = 0.0f;
	m_d3dViewport.MaxZ = 1.0f;

	m_renderSystem->addRenderView(this);
}

RenderViewWin32::~RenderViewWin32()
{
	close();
}

void RenderViewWin32::close()
{
	lostDevice();
	m_renderSystem->removeRenderView(this);
}

void RenderViewWin32::resize(int32_t width, int32_t height)
{
	HRESULT hr;

	T_ASSERT (m_renderStateStack.empty());

	if (!width || !height)
		return;

	if (m_d3dPresent.BackBufferWidth == width && m_d3dPresent.BackBufferHeight == height)
		return;

	m_d3dPresent.BackBufferWidth = width;
	m_d3dPresent.BackBufferHeight = height;

	m_d3dViewport.X = 0;
	m_d3dViewport.Y = 0;
	m_d3dViewport.Width = m_d3dPresent.BackBufferWidth;
	m_d3dViewport.Height = m_d3dPresent.BackBufferHeight;
	m_d3dViewport.MinZ = 0.0f;
	m_d3dViewport.MaxZ = 1.0f;

	hr = resetDevice(m_d3dDevice);
	T_FATAL_ASSERT_M (SUCCEEDED(hr), L"Failed to resize render view");
}

int RenderViewWin32::getWidth() const
{
	return m_d3dPresent.BackBufferWidth;
}

int RenderViewWin32::getHeight() const
{
	return m_d3dPresent.BackBufferHeight;
}

void RenderViewWin32::setViewport(const Viewport& viewport)
{
	if (m_renderStateStack.empty())
	{
		m_d3dViewport.X = viewport.left;
		m_d3dViewport.Y = viewport.top;
		m_d3dViewport.Width = viewport.width;
		m_d3dViewport.Height = viewport.height;
		m_d3dViewport.MinZ = viewport.nearZ;
		m_d3dViewport.MaxZ = viewport.farZ;
	}
	else
	{
		D3DVIEWPORT9& d3dViewport = m_renderStateStack.back().d3dViewport;

		d3dViewport.X = viewport.left;
		d3dViewport.Y = viewport.top;
		d3dViewport.Width = viewport.width;
		d3dViewport.Height = viewport.height;
		d3dViewport.MinZ = viewport.nearZ;
		d3dViewport.MaxZ = viewport.farZ;

		m_d3dDevice->SetViewport(&d3dViewport);
	}
}

Viewport RenderViewWin32::getViewport()
{
	const D3DVIEWPORT9& d3dViewport = m_renderStateStack.empty() ? m_d3dViewport : m_renderStateStack.back().d3dViewport;

	return Viewport(
		d3dViewport.X,
		d3dViewport.Y,
		d3dViewport.Width,
		d3dViewport.Height,
		d3dViewport.MinZ,
		d3dViewport.MaxZ
	);
}

bool RenderViewWin32::getNativeAspectRatio(float& outAspectRatio) const
{
	outAspectRatio = m_nativeAspectRatio;
	return true;
}

bool RenderViewWin32::begin()
{
	T_ASSERT (m_renderStateStack.empty());

	if (!m_renderSystem->beginRender())
		return false;

	T_ASSERT (m_d3dDevice);

	if (FAILED(m_d3dDevice->BeginScene()))
	{
		m_renderSystem->endRender(false);
		return false;
	}

	RenderState rs =
	{
		m_d3dViewport,
		m_d3dBackBuffer,
		m_d3dDepthStencilSurface
	};

	m_renderStateStack.push_back(rs);
	m_targetDirty = true;

	return true;
}

bool RenderViewWin32::begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil)
{
	T_ASSERT (!m_renderStateStack.empty());

	if (!m_d3dDevice || !renderTargetSet)
		return false;

	RenderTargetSetWin32* rts = static_cast< RenderTargetSetWin32* >(renderTargetSet);
	RenderTargetWin32* rt = rts->getRenderTarget(renderTarget);

	RenderState rs =
	{
		{ 0, 0, rts->getWidth(), rts->getHeight(), 0.0f, 1.0f },
		rt->getD3DColorSurface(),
		rts->getD3DDepthStencilSurface(),
		rt
	};

	if (keepDepthStencil)
		rs.d3dDepthStencilSurface = m_renderStateStack.back().d3dDepthStencilSurface;

	m_renderStateStack.push_back(rs);
	m_targetDirty = true;

	// Flag content valid here; it's not completely true but we don't want to store
	// target set as well in render state stack.
	rts->setContentValid(true);
	return true;
}

void RenderViewWin32::clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil)
{
	const RenderState& rs = m_renderStateStack.back();

	if (m_targetDirty)
		bindTargets();

	if (rs.renderTarget)
		rs.renderTarget->clear(
			m_d3dDevice,
			m_parameterCache,
			c_d3dClearMask[clearMask],
			color,
			depth,
			stencil
		);
	else
	{
		D3DCOLOR clearColor = D3DCOLOR_RGBA(
			uint8_t(color[0] * 255),
			uint8_t(color[1] * 255),
			uint8_t(color[2] * 255),
			uint8_t(color[3] * 255)
		);
		m_d3dDevice->Clear(0, NULL, c_d3dClearMask[clearMask], clearColor, depth, stencil);
	}
}

void RenderViewWin32::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	T_ASSERT (is_a< VertexBufferDx9 >(vertexBuffer));
	m_currentVertexBuffer = static_cast< VertexBufferDx9* >(vertexBuffer);
}

void RenderViewWin32::setIndexBuffer(IndexBuffer* indexBuffer)
{
	T_ASSERT (indexBuffer == 0 || is_a< IndexBufferDx9 >(indexBuffer));
	m_currentIndexBuffer = static_cast< IndexBufferDx9* >(indexBuffer);
}

void RenderViewWin32::setProgram(IProgram* program)
{
	T_ASSERT (is_a< ProgramWin32 >(program));
	m_currentProgram = static_cast< ProgramWin32* >(program);
}

void RenderViewWin32::draw(const Primitives& primitives)
{
	T_ASSERT (!m_renderStateStack.empty());
	T_ASSERT (m_currentProgram);
	T_ASSERT (m_currentVertexBuffer);

	if (m_targetDirty)
		bindTargets();

	if (!m_currentProgram->activate())
		return;

	VertexBufferDx9::activate(m_d3dDevice, m_currentVertexBuffer);
	IndexBufferDx9::activate(m_d3dDevice, m_currentIndexBuffer);

	if (primitives.indexed)
	{
		m_d3dDevice->DrawIndexedPrimitive(
			c_d3dPrimitiveType[primitives.type],
			0,
			primitives.minIndex,
			primitives.maxIndex - primitives.minIndex,
			primitives.offset,
			primitives.count
		);
	}
	else
	{
		m_d3dDevice->DrawPrimitive(
			c_d3dPrimitiveType[primitives.type],
			primitives.offset,
			primitives.count
		);
	}
}

void RenderViewWin32::end()
{
	T_ASSERT (!m_renderStateStack.empty());

	RenderState& rs = m_renderStateStack.back();
	if (rs.renderTarget)
		rs.renderTarget->resolve(m_d3dDevice);

	m_renderStateStack.pop_back();
	if (!m_renderStateStack.empty())
		m_targetDirty = true;
	else
		m_d3dDevice->EndScene();
}

void RenderViewWin32::present()
{
	HRESULT hr;

	hr = m_d3dSwapChain->Present(NULL, NULL, NULL, NULL, 0);

	if (SUCCEEDED(hr))
	{
		uint32_t previousQuery = (m_frameCount + 1) % sizeof_array(m_d3dSyncQueries);
		uint32_t currentQuery = m_frameCount % sizeof_array(m_d3dSyncQueries);

		m_d3dSyncQueries[currentQuery].get()->Issue(D3DISSUE_END);

		while (m_d3dSyncQueries[previousQuery]->GetData(NULL, 0, D3DGETDATA_FLUSH) == S_FALSE)
			;
	}

	m_renderSystem->endRender(
		hr == D3DERR_DEVICELOST ||
		hr == D3DERR_DEVICENOTRESET
	);
}

void RenderViewWin32::setMSAAEnable(bool msaaEnable)
{
	m_d3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, msaaEnable ? TRUE : FALSE);
}

void RenderViewWin32::pushMarker(const char* const marker)
{
	std::wstring wm = mbstows(marker); 
	D3DPERF_BeginEvent(D3DCOLOR_RGBA(255, 255, 255, 255), wm.c_str());
}

void RenderViewWin32::popMarker()
{
	D3DPERF_EndEvent();
}

HRESULT RenderViewWin32::lostDevice()
{
	log::debug << L"RenderViewWin32::lostDevice" << Endl;

	m_d3dDevice.release();
	m_d3dSwapChain.release();
	m_d3dBackBuffer.release();
	m_d3dDepthStencilSurface.release();

	for (uint32_t i = 0; i < sizeof_array(m_d3dSyncQueries); ++i)
		m_d3dSyncQueries[i].release();

	m_renderStateStack.clear();
	m_currentVertexBuffer = 0;
	m_currentIndexBuffer = 0;
	m_currentProgram = 0;

	return S_OK;
}

HRESULT RenderViewWin32::resetDevice(IDirect3DDevice9* d3dDevice)
{
	HRESULT hr;

	log::debug << L"RenderViewWin32::resetDevice, d3dDevice = " << (void*)(d3dDevice) << Endl;

	T_ASSERT (d3dDevice);
	m_d3dDevice = d3dDevice;

	if (m_d3dPresent.Windowed)
	{
		hr = m_d3dDevice->CreateAdditionalSwapChain(
			&m_d3dPresent,
			&m_d3dSwapChain.getAssign()
		);
		if (FAILED(hr))
		{
			log::error << L"Unable to create additional swap chain; hr = " << hr << Endl;
			return hr;
		}
	}
	else
	{
		hr = m_d3dDevice->GetSwapChain(
			0,
			&m_d3dSwapChain.getAssign()
		);
		if (FAILED(hr))
		{
			log::error << L"Reset device failed, unable to get primary swap chain" << Endl;
			return hr;
		}
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
			log::error << L"Failed to create query; hr = " << hr << Endl;
			return hr;
		}
	}

	return S_OK;
}

void RenderViewWin32::setD3DPresent(const D3DPRESENT_PARAMETERS& d3dPresent)
{
	m_d3dPresent = d3dPresent;
}

void RenderViewWin32::bindTargets()
{
	T_ASSERT (m_targetDirty);
	T_ASSERT (!m_renderStateStack.empty());
	HRESULT hr;

	// Ensure RT texture isn't bound to any sampler.
	for (int32_t i = 0; i < ParameterCache::MaxTextureCount; ++i)
	{
		m_parameterCache->setVertexTexture(i, 0);
		m_parameterCache->setPixelTexture(i, 0);
	}

	const RenderState& rs = m_renderStateStack.back();

	hr = m_d3dDevice->SetRenderTarget(0, rs.d3dBackBuffer);
	T_FATAL_ASSERT_M (SUCCEEDED(hr), L"SetRenderTarget failed");

	hr = m_d3dDevice->SetDepthStencilSurface(rs.d3dDepthStencilSurface);
	T_FATAL_ASSERT_M (SUCCEEDED(hr), L"SetDepthStencilSurface failed");

	hr = m_d3dDevice->SetViewport(&rs.d3dViewport);
	T_FATAL_ASSERT_M (SUCCEEDED(hr), L"SetViewport failed");

	m_targetDirty = false;
}

	}
}