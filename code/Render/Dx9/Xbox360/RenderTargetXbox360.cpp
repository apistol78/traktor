#include "Render/Dx9/Platform.h"
#include "Render/Dx9/Xbox360/RenderTargetXbox360.h"
#include "Render/Dx9/Xbox360/RenderSystemXbox360.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetXbox360", RenderTargetXbox360, ITexture)

RenderTargetXbox360::RenderTargetXbox360(RenderSystemXbox360* renderSystem)
:	m_width(0)
,	m_height(0)
,	m_d3dFormat(D3DFMT_UNKNOWN)
,	m_d3dMultisample(D3DMULTISAMPLE_NONE)
,	m_acquireWidth(0)
,	m_acquireHeight(0)
,	m_tileCount(0)
,	m_acquiredTarget(0)
{
}

RenderTargetXbox360::~RenderTargetXbox360()
{
	destroy();
}

bool RenderTargetXbox360::create(
	IDirect3DDevice9* d3dDevice,
	RenderTargetPool* pool,
	int width,
	int height,
	D3DFORMAT d3dFormat,
	D3DMULTISAMPLE_TYPE d3dMultisample,
	bool attachDepthStencil,
	bool doubleBuffered
)
{
	HRESULT hr;

	m_pool = pool;
	m_width = width;
	m_height = height;
	m_d3dFormat = d3dFormat;
	m_d3dMultisample = d3dMultisample;

	hr = d3dDevice->CreateTexture(
		m_width,
		m_height,
		1,
		0,
		m_d3dFormat,
		D3DPOOL_DEFAULT,
		&m_d3dTargetTexture.getAssign(),
		NULL
	);
	T_ASSERT_M (SUCCEEDED(hr), L"Unable to create target texture");

	// Create a secondary texture with the same size,
	// used when tiling back to primary render target.
	if (doubleBuffered)
	{
		hr = d3dDevice->CreateTexture(
			m_width,
			m_height,
			1,
			0,
			m_d3dFormat,
			D3DPOOL_DEFAULT,
			&m_d3dSecondaryTargetTexture.getAssign(),
			NULL
		);
		T_ASSERT_M (SUCCEEDED(hr), L"Unable to create secondary target texture");
	}

	// Prepare tiling if necessary.
	uint32_t surfaceSize = XGSurfaceSize(width, height, m_d3dFormat, m_d3dMultisample);
	if (surfaceSize >= (GPU_EDRAM_TILES * 1) / 4)
	{
		if (surfaceSize >= (GPU_EDRAM_TILES * 2) / 4)
			m_tileCount = 3;
		else
			m_tileCount = 2;

		uint32_t tileWidth = width;
		uint32_t tileHeight = height / m_tileCount;
		
		for (int i = 0; i < m_tileCount; ++i)
		{
			m_tileRects[i].x1 = 0;
			m_tileRects[i].y1 = tileHeight * i;
			m_tileRects[i].x2 = tileWidth;
			m_tileRects[i].y2 = tileHeight * (i + 1);
		}

		tileWidth = XGNextMultiple(tileWidth, GPU_EDRAM_TILE_WIDTH_1X);
		tileHeight = XGNextMultiple(tileHeight, GPU_EDRAM_TILE_HEIGHT_1X);

		tileWidth = XGNextMultiple(tileWidth, GPU_TEXTURE_TILE_DIMENSION);
		tileHeight = XGNextMultiple(tileHeight, GPU_TEXTURE_TILE_DIMENSION);

		m_acquireWidth = int(tileWidth);
		m_acquireHeight = int(tileHeight);
	}
	else
	{
		// No tiling required.

		m_tileCount = 0;

		m_acquireWidth = m_width;
		m_acquireHeight = m_height;
	}

	if (attachDepthStencil)
	{
		D3DSURFACE_PARAMETERS d3dsp;
		memset (&d3dsp, 0, sizeof(d3dsp));
		d3dsp.Base = GPU_EDRAM_TILES - XGSurfaceSize(m_acquireWidth, m_acquireHeight, D3DFMT_D24FS8, m_d3dMultisample);
		d3dsp.HierarchicalZBase = 0;

		hr = d3dDevice->CreateDepthStencilSurface(
			m_acquireWidth,
			m_acquireHeight,
			D3DFMT_D24FS8,
			m_d3dMultisample,
			0,
			TRUE,
			&m_d3dTargetDepthStencilSurface.getAssign(),
			&d3dsp
		);
		T_ASSERT_M (SUCCEEDED(hr), L"Unable to create depth-stencil surface");
	}

	return true;
}

void RenderTargetXbox360::destroy()
{
	//getRenderSystem()->releaseResource(m_d3dTargetTexture);
	//getRenderSystem()->releaseResource(m_d3dSecondaryTargetTexture);
	//getRenderSystem()->releaseResource(m_d3dTargetDepthStencilSurface);
}

ITexture* RenderTargetXbox360::resolve()
{
	return this;
}

int RenderTargetXbox360::getWidth() const
{
	return m_width;
}

int RenderTargetXbox360::getHeight() const
{
	return m_height;
}

int RenderTargetXbox360::getDepth() const
{
	return 1;
}

IDirect3DBaseTexture9* RenderTargetXbox360::getD3DBaseTexture() const
{
	return m_d3dTargetTexture;
}

bool RenderTargetXbox360::begin(IDirect3DDevice9* d3dDevice)
{
	T_ASSERT (!m_acquiredTarget);

#if defined(_XBOX)
	PIXSetMarker(0xffff00, "RT::begin %p (%d tiles)", this, m_tileCount);
#endif

	// @hack Use lesser format for FP-render targets as they are the only
	// ones which support high-precision blending.
	D3DFORMAT d3dFormatAcquire = m_d3dFormat;
	if (m_d3dFormat == D3DFMT_A16B16G16R16F || m_d3dFormat == D3DFMT_A32B32G32R32F)
		d3dFormatAcquire = D3DFMT_A2B10G10R10F_EDRAM;

	m_acquiredTarget = m_pool->acquire(d3dDevice, m_acquireWidth, m_acquireHeight, d3dFormatAcquire, m_d3dMultisample);
	if (!m_acquiredTarget)
		return false;

	d3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	d3dDevice->SetRenderState(D3DRS_HIGHPRECISIONBLENDENABLE, d3dFormatAcquire != m_d3dFormat ? TRUE : FALSE);
	d3dDevice->SetRenderTarget(0, m_acquiredTarget->d3dSurface);
	d3dDevice->SetDepthStencilSurface(m_d3dTargetDepthStencilSurface);

	if (m_tileCount > 0)
	{
		const D3DVECTOR4 clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		const float clearDepth = 0.0f;
		d3dDevice->BeginTiling(
			0,
			m_tileCount,
			m_tileRects,
			&clearColor,
			clearDepth,
			0
		);
	}

	return true;
}

void RenderTargetXbox360::end(IDirect3DDevice9* d3dDevice)
{
	T_ASSERT (m_acquiredTarget);
	
	HRESULT hr;
	if (m_tileCount > 0)
	{
		hr = d3dDevice->EndTiling(
			D3DRESOLVE_RENDERTARGET0,
			NULL,
			m_d3dTargetTexture,
			NULL,
			1.0f,
			0,
			NULL
		);
	}
	else
	{
		hr = d3dDevice->Resolve(
			D3DRESOLVE_RENDERTARGET0,
			NULL,
			m_d3dTargetTexture,
			NULL,
			0,
			0,
			NULL,
			0.0f,
			0,
			NULL
		);
	}
	T_ASSERT (SUCCEEDED(hr));

	d3dDevice->SetRenderTarget(0, 0);
	d3dDevice->SetRenderState(D3DRS_HIGHPRECISIONBLENDENABLE, FALSE);

	m_pool->release(m_acquiredTarget);

#if defined(_XBOX)
	PIXSetMarker(0xff00ff, "RT::end %p", this);
#endif
}

void RenderTargetXbox360::cycleTargets()
{
	ComRef< IDirect3DTexture9 > d3dTemporary = m_d3dSecondaryTargetTexture;
	m_d3dSecondaryTargetTexture = m_d3dTargetTexture;
	m_d3dTargetTexture = m_d3dSecondaryTargetTexture;

#if defined(_XBOX)
	PIXSetMarker(0x0000ff, "RT::cycle %p", this);
#endif
}

	}
}
