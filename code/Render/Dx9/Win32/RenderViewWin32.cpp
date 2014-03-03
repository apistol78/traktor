#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/Dx9/ClearTarget.h"
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
	ClearTarget* clearTarget,
	ParameterCache* parameterCache,
	IDirect3DDevice9* d3dDevice
)
:	m_renderSystem(renderSystem)
,	m_clearTarget(clearTarget)
,	m_parameterCache(parameterCache)
,	m_d3dDevice(d3dDevice)
,	m_frameCount(0)
,	m_targetDirty(false)
,	m_drawCalls(0)
,	m_primitiveCount(0)
{
	m_renderSystem->addRenderView(this);
}

void RenderViewWin32::close()
{
	m_renderSystem->removeRenderView(this);
}

bool RenderViewWin32::setGamma(float gamma)
{
	return false;
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

bool RenderViewWin32::begin(EyeType eye)
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
		{ getWidth(), getHeight() },
		{ m_d3dBackBuffer, 0 },
		m_d3dDepthStencilSurface,
		{ 0, 0 }
	};

	m_renderStateStack.push_back(rs);
	m_targetDirty = true;
	m_drawCalls = 0;
	m_primitiveCount = 0;

	return true;
}

bool RenderViewWin32::begin(RenderTargetSet* renderTargetSet)
{
	T_ASSERT (!m_renderStateStack.empty());

	if (!m_d3dDevice || !renderTargetSet)
		return false;

	RenderTargetSetWin32* rts = static_cast< RenderTargetSetWin32* >(renderTargetSet);

	if (rts->getRenderTargetCount() >= 2)
	{
		RenderTargetWin32* rt0 = rts->getRenderTarget(0);
		RenderTargetWin32* rt1 = rts->getRenderTarget(1);

		RenderState rs =
		{
			{ 0, 0, rts->getWidth(), rts->getHeight(), 0.0f, 1.0f },
			{ rts->getWidth(), rts->getHeight() },
			{ rt0->getD3DColorSurface(), rt1->getD3DColorSurface() },
			rts->getD3DDepthStencilSurface(),
			{ rt0, rt1 }
		};

		if (rts->usingPrimaryDepthStencil())
			rs.d3dDepthStencilSurface = m_d3dDepthStencilSurface;

		m_renderStateStack.push_back(rs);
		m_targetDirty = true;

		// Flag content valid here; it's not completely true but we don't want to store
		// target set as well in render state stack.
		rts->setContentValid(true);
		return true;
	}
	else
	{
		// Only a single target.
		return begin(renderTargetSet, 0);
	}
}

bool RenderViewWin32::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	T_ASSERT (!m_renderStateStack.empty());

	if (!m_d3dDevice || !renderTargetSet)
		return false;

	RenderTargetSetWin32* rts = static_cast< RenderTargetSetWin32* >(renderTargetSet);
	RenderTargetWin32* rt = rts->getRenderTarget(renderTarget);

	RenderState rs =
	{
		{ 0, 0, rts->getWidth(), rts->getHeight(), 0.0f, 1.0f },
		{ rt->getWidth(), rt->getHeight() },
		{ rt->getD3DColorSurface(), 0 },
		rts->getD3DDepthStencilSurface(),
		{ rt, 0 }
	};

	if (rts->usingPrimaryDepthStencil())
		rs.d3dDepthStencilSurface = m_d3dDepthStencilSurface;

	m_renderStateStack.push_back(rs);
	m_targetDirty = true;

	// Flag content valid here; it's not completely true but we don't want to store
	// target set as well in render state stack.
	rts->setContentValid(true);
	return true;
}

void RenderViewWin32::clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil)
{
	T_ASSERT (colors);
	const RenderState& rs = m_renderStateStack.back();

	if (m_targetDirty)
		bindTargets();

	if (clearMask & CfColor)
	{
		if (rs.renderTarget[0] && rs.renderTarget[1])
		{
			m_clearTarget->clear(m_d3dDevice, m_parameterCache, rs.targetSize[0], rs.targetSize[1], colors, 2);
			clearMask &= ~CfColor;
		}
		else if (rs.renderTarget[0])
		{
			// Perform slow clear only on single render targets with FP formats.
			if (rs.renderTarget[0]->getD3DFormat() >= D3DFMT_R16F)
			{
				m_clearTarget->clear(m_d3dDevice, m_parameterCache, rs.targetSize[0], rs.targetSize[1], colors, 1);
				clearMask &= ~CfColor;
			}
		}
	}

	if (c_d3dClearMask[clearMask])
	{
		D3DCOLOR clearColor = D3DCOLOR_RGBA(
			uint8_t(colors[0].getRed() * 255),
			uint8_t(colors[0].getGreen() * 255),
			uint8_t(colors[0].getBlue() * 255),
			uint8_t(colors[0].getAlpha() * 255)
		);
		m_d3dDevice->Clear(0, NULL, c_d3dClearMask[clearMask], clearColor, depth, stencil);
	}
}

void RenderViewWin32::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	T_ASSERT (is_a< VertexBufferDx9 >(vertexBuffer));
	T_ASSERT (indexBuffer == 0 || is_a< IndexBufferDx9 >(indexBuffer));
	T_ASSERT (is_a< ProgramWin32 >(program));
	T_ASSERT (!m_renderStateStack.empty());

	VertexBufferDx9* vertexBufferDx9 = static_cast< VertexBufferDx9* >(vertexBuffer);
	IndexBufferDx9* indexBufferDx9 = static_cast< IndexBufferDx9* >(indexBuffer);
	ProgramWin32* programDx9 = static_cast< ProgramWin32* >(program);

	if (m_targetDirty)
		bindTargets();

	if (!programDx9->activate())
		return;

	vertexBufferDx9->activate(m_d3dDevice);

	IndexBufferDx9::activate(m_d3dDevice, indexBufferDx9);

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

	m_drawCalls++;
	m_primitiveCount += primitives.count;
}

void RenderViewWin32::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
}

void RenderViewWin32::end()
{
	T_ASSERT (!m_renderStateStack.empty());

	RenderState& rs = m_renderStateStack.back();
	if (rs.renderTarget[0])
		rs.renderTarget[0]->resolve(m_d3dDevice);
	if (rs.renderTarget[1])
		rs.renderTarget[1]->resolve(m_d3dDevice);

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

	if (SUCCEEDED(hr) && m_d3dSyncQueries[0])
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

void RenderViewWin32::pushMarker(const char* const marker)
{
	std::wstring wm = marker ? mbstows(marker) : L"Unnamed"; 
	D3DPERF_BeginEvent(D3DCOLOR_RGBA(255, 255, 255, 255), wm.c_str());
}

void RenderViewWin32::popMarker()
{
	D3DPERF_EndEvent();
}

void RenderViewWin32::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = m_drawCalls;
	outStatistics.primitiveCount = m_primitiveCount;
}

bool RenderViewWin32::getBackBufferContent(void* buffer) const
{
	return false;
}

void RenderViewWin32::bindTargets()
{
	T_ASSERT (m_targetDirty);
	T_ASSERT (!m_renderStateStack.empty());
	HRESULT hr;

	// Ensure RT texture isn't bound to any sampler.
	for (int32_t i = 0; i < ParameterCache::VertexTextureCount; ++i)
		m_parameterCache->setVertexTexture(i, 0);
	for (int32_t i = 0; i < ParameterCache::PixelTextureCount; ++i)
		m_parameterCache->setPixelTexture(i, 0);

	const RenderState& rs = m_renderStateStack.back();

	hr = m_d3dDevice->SetRenderTarget(0, rs.d3dColorBuffer[0]);
	T_FATAL_ASSERT_M (SUCCEEDED(hr), L"SetRenderTarget failed");

	hr = m_d3dDevice->SetRenderTarget(1, rs.d3dColorBuffer[1]);
	T_FATAL_ASSERT_M (SUCCEEDED(hr), L"SetRenderTarget failed");

	hr = m_d3dDevice->SetDepthStencilSurface(rs.d3dDepthStencilSurface);
	T_FATAL_ASSERT_M (SUCCEEDED(hr), L"SetDepthStencilSurface failed");

	hr = m_d3dDevice->SetViewport(&rs.d3dViewport);
	T_FATAL_ASSERT_M (SUCCEEDED(hr), L"SetViewport failed");

	// Expose target size to shaders.
	const float T_ALIGN16 targetSize[] = { float(rs.targetSize[0]), float(rs.targetSize[1]), 0.0f, 0.0f };
	m_parameterCache->setVertexShaderConstant(0, 1, targetSize);
	m_parameterCache->setPixelShaderConstant(0, 1, targetSize);

	m_targetDirty = false;
}

	}
}