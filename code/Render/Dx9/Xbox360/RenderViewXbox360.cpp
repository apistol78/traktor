#include "Render/Dx9/Xbox360/RenderViewXbox360.h"
#include "Render/Dx9/Xbox360/RenderSystemXbox360.h"
#include "Render/Dx9/Xbox360/RenderTargetSetXbox360.h"
#include "Render/Dx9/Xbox360/RenderTargetXbox360.h"
#include "Render/Dx9/Xbox360/ProgramXbox360.h"
#include "Render/Dx9/IndexBufferDx9.h"
#include "Render/Dx9/VertexBufferDx9.h"
#include "Core/Log/Log.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewXbox360", RenderViewXbox360, RenderView)

RenderViewXbox360::RenderViewXbox360(
	const RenderViewCreateDesc& createDesc,
	RenderSystemXbox360* renderSystem,
	IDirect3DDevice9* d3dDevice,
	RenderTargetPool* renderTargetPool,
	int width,
	int height,
	D3DFORMAT d3dPrimaryTargetFormat
)
:	m_createDesc(createDesc)
,	m_renderSystem(renderSystem)
,	m_d3dDevice(d3dDevice)
,	m_renderTargetPool(renderTargetPool)
,	m_currentRenderState(0)
{
	m_renderTargetPrimary = gc_new< RenderTargetXbox360 >(renderSystem);
	m_renderTargetPrimary->create(m_d3dDevice, m_renderTargetPool, width, height, d3dPrimaryTargetFormat, D3DMULTISAMPLE_NONE, true, true);

	m_d3dViewport.X = 0;
	m_d3dViewport.Y = 0;
	m_d3dViewport.Width = width;
	m_d3dViewport.Height = height;
	m_d3dViewport.MinZ = 0.0f;
	m_d3dViewport.MaxZ = 1.0f;
}

RenderViewXbox360::~RenderViewXbox360()
{
	close();
}

void RenderViewXbox360::close()
{
	T_ASSERT (m_renderStateStack.empty());
	m_renderTargetPrimary = 0;
	m_renderTargetPool = 0;
	m_d3dDevice.release();
}

void RenderViewXbox360::resize(int32_t width, int32_t height)
{
}

void RenderViewXbox360::setViewport(const Viewport& viewport)
{
	if (m_renderStateStack.empty())
	{
		m_d3dViewport.X = viewport.left;
		m_d3dViewport.Y = viewport.top;
		m_d3dViewport.Width = viewport.width;
		m_d3dViewport.Height = viewport.height;
		m_d3dViewport.MinZ = 1.0f - viewport.nearZ;
		m_d3dViewport.MaxZ = 1.0f - viewport.farZ;
	}
	else
	{
		D3DVIEWPORT9& d3dViewport = m_renderStateStack.back().d3dViewport;

		d3dViewport.X = viewport.left;
		d3dViewport.Y = viewport.top;
		d3dViewport.Width = viewport.width;
		d3dViewport.Height = viewport.height;
		d3dViewport.MinZ = 1.0f - viewport.nearZ;
		d3dViewport.MaxZ = 1.0f - viewport.farZ;

		m_d3dDevice->SetViewport(&d3dViewport);
	}
}

Viewport RenderViewXbox360::getViewport()
{
	const D3DVIEWPORT9& d3dViewport = m_renderStateStack.empty() ? m_d3dViewport : m_renderStateStack.back().d3dViewport;

	return Viewport(
		d3dViewport.X,
		d3dViewport.Y,
		d3dViewport.Width,
		d3dViewport.Height,
		1.0f - d3dViewport.MinZ,
		1.0f - d3dViewport.MaxZ
	);
}

bool RenderViewXbox360::begin()
{
	T_ASSERT (m_d3dDevice);
	T_ASSERT (!m_currentRenderState);

	// Measured by PIX, how can we do this automatically?
	m_d3dDevice->GetShaderGPRAllocation(&m_defaultGPR[0], &m_defaultGPR[1], &m_defaultGPR[2]);
	m_d3dDevice->SetShaderGPRAllocation(0, 32, 96);

	RenderState rs =
	{
		m_d3dViewport,
		m_renderTargetPrimary
	};
	m_renderStateStack.push_back(rs);

	return true;
}

bool RenderViewXbox360::begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil)
{
	T_ASSERT (m_d3dDevice);
	
	//RenderTargetSetXbox360* rts = static_cast< RenderTargetSetXbox360* >(renderTargetSet);
	//RenderTargetXbox360* rt = rts->getRenderTarget(renderTarget);

	//RenderState rs =
	//{
	//	{ 0, 0, renderTarget->getWidth(), renderTarget->getHeight(), 1.0f, 0.0f },
	//	rt
	//};
	//m_renderStateStack.push_back(rs);

	return true;
}

void RenderViewXbox360::clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil)
{
}

void RenderViewXbox360::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	T_ASSERT (is_a< VertexBufferDx9 >(vertexBuffer));
	m_currentVertexBuffer = static_cast< VertexBufferDx9* >(vertexBuffer);
}

void RenderViewXbox360::setIndexBuffer(IndexBuffer* indexBuffer)
{
	T_ASSERT (indexBuffer == 0 || is_a< IndexBufferDx9 >(indexBuffer));
	m_currentIndexBuffer = static_cast< IndexBufferDx9* >(indexBuffer);
}

void RenderViewXbox360::setProgram(Program* program)
{
	T_ASSERT (is_a< ProgramXbox360 >(program));
	m_currentProgram = static_cast< ProgramXbox360* >(program);
}

void RenderViewXbox360::draw(const Primitives& primitives)
{
	T_ASSERT (!m_renderStateStack.empty());

	RenderState& rs = m_renderStateStack.back();
	if (m_currentRenderState != &rs)
	{
		if (m_currentRenderState)
			m_currentRenderState->renderTarget->end(m_d3dDevice);
		if (!rs.renderTarget->begin(m_d3dDevice))
		{
			m_currentRenderState = 0;
			return;
		}
		m_d3dDevice->SetViewport(&rs.d3dViewport);
		m_currentRenderState = &rs;
	}

	m_currentVertexBuffer->activate(m_d3dDevice);
	m_currentIndexBuffer->activate(m_d3dDevice);
	m_currentProgram->activate();

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

void RenderViewXbox360::end()
{
	T_ASSERT (!m_renderStateStack.empty());

	RenderState& rs = m_renderStateStack.back();
	if (m_currentRenderState == &rs)
	{
		rs.renderTarget->end(m_d3dDevice);
		m_currentRenderState = 0;
	}

	m_renderStateStack.pop_back();

	if (!m_renderStateStack.empty())
	{
		const RenderState& rs = m_renderStateStack.back();
		m_d3dDevice->SetViewport(&rs.d3dViewport);
	}
}

void RenderViewXbox360::present()
{
	T_ASSERT (!m_currentRenderState);

	// Restore default GPR allocation as stated in the documentation.
	m_d3dDevice->SetShaderGPRAllocation(m_defaultGPR[0], m_defaultGPR[1], m_defaultGPR[2]);

	m_d3dDevice->SynchronizeToPresentationInterval();
	m_d3dDevice->Swap(
		m_renderTargetPrimary->getD3DBaseTexture(),
		NULL
	);

	m_renderTargetPrimary->cycleTargets();

	// Cleanup deferred resources.
	//m_renderSystem->cleanupResources();
}

	}
}
