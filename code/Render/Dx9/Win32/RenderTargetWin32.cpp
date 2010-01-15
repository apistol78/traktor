#include "Render/Types.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Dx9/ContextDx9.h"
#include "Render/Dx9/ParameterCache.h"
#include "Render/Dx9/VertexBufferDx9.h"
#include "Render/Dx9/Win32/RenderTargetWin32.h"
#include "Render/Dx9/Win32/ProgramWin32.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct ClearTriangleVertex
{
	float pos[3];
};

const ClearTriangleVertex c_clearTriangles[] =
{
	{ -2.0f,  5.0f, 0.0f },
	{  5.0f, -2.0f, 0.0f },
	{ -2.0f, -2.0f, 0.0f }
};

const char c_clearEffect[] =
{
	"float4 g_ClearColor;								\r\n"

	"float4 vs_main(float3 pos : POSITION) : POSITION	\r\n"
	"{													\r\n"
	"	return float4(pos.x, pos.y, 1.0f, 1.0f);		\r\n"
	"}													\r\n"

	"float4 ps_main() : COLOR0							\r\n"
	"{													\r\n"
	"	return g_ClearColor;							\r\n"
	"}													\r\n"

	"technique Clear									\r\n"
	"{													\r\n"
	"	pass Clear										\r\n"
	"	{												\r\n"
	"		VertexShader = compile vs_2_0 vs_main();	\r\n"
	"		PixelShader = compile ps_2_0 ps_main();		\r\n"
	"	}												\r\n"
	"}													\r\n"
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetWin32", RenderTargetWin32, ITexture)

RenderTargetWin32::RenderTargetWin32(ContextDx9* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
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

	if (!setDesc.multiSample)
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
			return false;

		hr = m_d3dTargetTexture->GetSurfaceLevel(0, &m_d3dTargetSurface.getAssign());
		if (FAILED(hr))
			return false;
	}
	else
	{
		hr = d3dDevice->CreateRenderTarget(
			setDesc.width,
			setDesc.height,
			d3dFormat,
			c_d3dMultiSample[setDesc.multiSample],
			0,
			FALSE,
			&m_d3dTargetSurface.getAssign(),
			NULL
		);
		if (FAILED(hr))
			return false;

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
			return false;

		hr = m_d3dTargetTexture->GetSurfaceLevel(0, &m_d3dResolveTargetSurface.getAssign());
		if (FAILED(hr))
			return false;
	}

	m_width = setDesc.width;
	m_height = setDesc.height;

	if (targetDesc.format >= TfR16G16B16A16F)
	{
		HRESULT hr = D3DXCreateEffect(
			d3dDevice,
			c_clearEffect,
			sizeof(c_clearEffect),
			NULL,
			NULL,
			D3DXSHADER_SKIPVALIDATION,
			NULL,
			&m_d3dClearEffect.getAssign(),
			NULL
		);
		if (FAILED(hr))
			return false;

		T_ASSERT (m_d3dClearEffect);
		m_d3dClearTechnique = m_d3dClearEffect->GetTechniqueByName("Clear");
		m_d3dClearColor = m_d3dClearEffect->GetParameterByName(NULL, "g_ClearColor");

		hr = m_d3dClearEffect->SetTechnique(m_d3dClearTechnique);
		T_ASSERT (SUCCEEDED(hr));
	}

	return true;
}

void RenderTargetWin32::destroy()
{
	if (!m_context)
		return;
	m_context->releaseComRef(m_d3dClearEffect);
	m_context->releaseComRef(m_d3dTargetTexture);
	m_context->releaseComRef(m_d3dTargetSurface);
	m_context->releaseComRef(m_d3dResolveTargetSurface);
}

int RenderTargetWin32::getWidth() const
{
	return m_width;
}

int RenderTargetWin32::getHeight() const
{
	return m_height;
}

int RenderTargetWin32::getDepth() const
{
	return 1;
}

IDirect3DBaseTexture9* RenderTargetWin32::getD3DBaseTexture() const
{
	return m_d3dTargetTexture;
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

void RenderTargetWin32::clear(
	IDirect3DDevice9* d3dDevice,
	ParameterCache* parameterCache,
	DWORD flags,
	const float color[4],
	float z,
	DWORD stencil
)
{
	if (m_d3dClearEffect)
	{
		D3DVIEWPORT9 d3dvp0, d3dvp;
		UINT passes;
		HRESULT hr;

		// Clear stencil and z using ordinary clear call.
		if (flags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL))
			d3dDevice->Clear(0, NULL, flags, 0, z, stencil);

		if (!(flags & D3DCLEAR_TARGET))
			return;

		// Save current viewport.
		hr = d3dDevice->GetViewport(&d3dvp0);
		T_ASSERT (SUCCEEDED(hr));

		d3dvp.X = 0;
		d3dvp.Y = 0;
		d3dvp.Width = m_width;
		d3dvp.Height = m_height;
		d3dvp.MinZ = 0.0f;
		d3dvp.MaxZ = 1.0f;
		hr = d3dDevice->SetViewport(&d3dvp);
		T_ASSERT (SUCCEEDED(hr));

		hr = m_d3dClearEffect->Begin(&passes, D3DXFX_DONOTSAVESTATE);
		T_ASSERT (SUCCEEDED(hr));
		if (FAILED(hr))
			return;

		// Use our own state cache to setup render states.
		parameterCache->setRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		parameterCache->setRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		parameterCache->setRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		parameterCache->setRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
		parameterCache->setRenderState(D3DRS_STENCILENABLE, FALSE);
		parameterCache->setRenderState(D3DRS_ZENABLE, FALSE);
		parameterCache->setRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
		parameterCache->setRenderState(D3DRS_ZWRITEENABLE, FALSE);

		m_d3dClearEffect->BeginPass(0);

		D3DXVECTOR4 clearColor(
			color[0],
			color[1],
			color[2],
			color[3]
		);
		hr = m_d3dClearEffect->SetVector(m_d3dClearColor, &clearColor);
		T_ASSERT (SUCCEEDED(hr));

		hr = d3dDevice->SetFVF(D3DFVF_XYZ);
		T_ASSERT (SUCCEEDED(hr));

		hr = d3dDevice->DrawPrimitiveUP(
			D3DPT_TRIANGLELIST,
			1,
			c_clearTriangles,
			sizeof(ClearTriangleVertex)
		);
		T_ASSERT (SUCCEEDED(hr));

		hr = m_d3dClearEffect->EndPass();
		T_ASSERT (SUCCEEDED(hr));

		hr = m_d3dClearEffect->End();
		T_ASSERT (SUCCEEDED(hr));

		hr = d3dDevice->SetViewport(&d3dvp0);
		T_ASSERT (SUCCEEDED(hr));

		// Need to force to dirty as various states may have been trashed.
		ProgramWin32::forceDirty();
		VertexBufferDx9::forceDirty();
	}
	else
	{
		// Non-FP targets can be cleared by the normal path.
		D3DCOLOR clearColor = D3DCOLOR_RGBA(
			uint8_t(color[0] * 255),
			uint8_t(color[1] * 255),
			uint8_t(color[2] * 255),
			uint8_t(color[3] * 255)
		);
		d3dDevice->Clear(0, NULL, flags, clearColor, z, stencil);
	}
}

	}
}
