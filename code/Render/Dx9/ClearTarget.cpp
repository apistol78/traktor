#include "Core/Log/Log.h"
#include "Render/Dx9/ClearTarget.h"
#include "Render/Dx9/ParameterCache.h"

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
	"float4 g_ClearColor_0 : register(c1);				\r\n"
	"float4 g_ClearColor_1 : register(c2);				\r\n"

	"float4 vs_main(float3 pos : POSITION) : POSITION	\r\n"
	"{													\r\n"
	"	return float4(pos.x, pos.y, 1.0f, 1.0f);		\r\n"
	"}													\r\n"

	"void ps_main_1(									\r\n"
	"	out float4 C0 : COLOR0							\r\n"
	")													\r\n"
	"{													\r\n"
	"	C0 = g_ClearColor_0;							\r\n"
	"}													\r\n"

	"void ps_main_2(									\r\n"
	"	out float4 C0 : COLOR0,							\r\n"
	"	out float4 C1 : COLOR1							\r\n"
	")													\r\n"
	"{													\r\n"
	"	C0 = g_ClearColor_0;							\r\n"
	"	C1 = g_ClearColor_1;							\r\n"
	"}													\r\n"

	"technique Clear_1									\r\n"
	"{													\r\n"
	"	pass Clear										\r\n"
	"	{												\r\n"
	"		VertexShader = compile vs_2_0 vs_main();	\r\n"
	"		PixelShader = compile ps_2_0 ps_main_1();	\r\n"
	"	}												\r\n"
	"}													\r\n"

	"technique Clear_2									\r\n"
	"{													\r\n"
	"	pass Clear										\r\n"
	"	{												\r\n"
	"		VertexShader = compile vs_2_0 vs_main();	\r\n"
	"		PixelShader = compile ps_2_0 ps_main_2();	\r\n"
	"	}												\r\n"
	"}													\r\n"
};

class StateManagerIntercept : public ID3DXEffectStateManager
{
public:
	StateManagerIntercept(ParameterCache* parameterCache)
	:	m_parameterCache(parameterCache)
	{
	}

	STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) { return S_OK; }
	STDMETHOD_(ULONG, AddRef)(THIS) { return S_OK; }
	STDMETHOD_(ULONG, Release)(THIS) { return S_OK; }

	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix) { T_BREAKPOINT; return S_FALSE; }
	STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9 *pMaterial) { T_BREAKPOINT; return S_FALSE; }
	STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT9 *pLight) { T_BREAKPOINT; return S_FALSE; }
	STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable) { T_BREAKPOINT; return S_FALSE; }
	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value) { T_BREAKPOINT; return S_FALSE; }
	STDMETHOD(SetTexture)(THIS_ DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture) { T_BREAKPOINT; return S_FALSE; }
#if defined(_XBOX)
	STDMETHOD(Do_Not_Use_SetTextureStageState)(THIS_ DWORD Stage, DWORD Type, DWORD Value) { T_BREAKPOINT; return S_FALSE; }
#else
	STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) { T_BREAKPOINT; return S_FALSE; }
#endif
	STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) { T_BREAKPOINT; return S_FALSE; }
	STDMETHOD(SetNPatchMode)(THIS_ FLOAT NumSegments) { T_BREAKPOINT; return S_FALSE; }
	STDMETHOD(SetFVF)(THIS_ DWORD FVF) { T_BREAKPOINT; return S_FALSE; }

	STDMETHOD(SetVertexShader)(THIS_ LPDIRECT3DVERTEXSHADER9 pShader) { m_parameterCache->setVertexShader(pShader); return S_OK; }
	STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount) { m_parameterCache->setVertexShaderConstantAlways(RegisterIndex, RegisterCount, pConstantData); return S_OK; }
	STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount) { T_BREAKPOINT; return S_FALSE; }
	STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount) { T_BREAKPOINT; return S_FALSE; }

	STDMETHOD(SetPixelShader)(THIS_ LPDIRECT3DPIXELSHADER9 pShader) { m_parameterCache->setPixelShader(pShader); return S_OK; }
	STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount) { m_parameterCache->setPixelShaderConstantAlways(RegisterIndex, RegisterCount, pConstantData); return S_OK; }
	STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount) { T_BREAKPOINT; return S_FALSE; }
	STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount) { T_BREAKPOINT; return S_FALSE; }

private:
	ParameterCache* m_parameterCache;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ClearTarget", ClearTarget, Object)

bool ClearTarget::create(IDirect3DDevice9* d3dDevice)
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
	{
		log::error << L"Render target create failed; Unable to create clear effect" << Endl;
		return false;
	}

	T_ASSERT (m_d3dClearEffect);
	m_d3dClearTechnique[0] = m_d3dClearEffect->GetTechniqueByName("Clear_1");
	m_d3dClearTechnique[1] = m_d3dClearEffect->GetTechniqueByName("Clear_2");
	m_d3dClearColor[0] = m_d3dClearEffect->GetParameterByName(NULL, "g_ClearColor_0");
	m_d3dClearColor[1] = m_d3dClearEffect->GetParameterByName(NULL, "g_ClearColor_1");

	return true;
}

bool ClearTarget::clear(IDirect3DDevice9* d3dDevice, ParameterCache* parameterCache, int32_t width, int32_t height, const Color4f* colors, int32_t ntargets)
{
	T_ASSERT (ntargets > 0 && ntargets <= sizeof_array(m_d3dClearTechnique));

	HRESULT hr;
	D3DVIEWPORT9 d3dvp0, d3dvp;
	UINT passes;

	//// Clear stencil and z using ordinary clear call.
	//if (flags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL))
	//	d3dDevice->Clear(0, NULL, flags, 0, z, stencil);

	//if (!(flags & D3DCLEAR_TARGET))
	//	return;

	// Save current viewport.
	hr = d3dDevice->GetViewport(&d3dvp0);
	T_ASSERT (SUCCEEDED(hr));

	d3dvp.X = 0;
	d3dvp.Y = 0;
	d3dvp.Width = width;
	d3dvp.Height = height;
	d3dvp.MinZ = 0.0f;
	d3dvp.MaxZ = 1.0f;
	hr = d3dDevice->SetViewport(&d3dvp);
	T_ASSERT (SUCCEEDED(hr));

	// Replace with our own state manager; need to track everything.
	StateManagerIntercept sm(parameterCache);
	m_d3dClearEffect->SetStateManager(&sm);

	hr = m_d3dClearEffect->SetTechnique(m_d3dClearTechnique[ntargets - 1]);
	T_ASSERT (SUCCEEDED(hr));
	if (FAILED(hr))
		return false;

	hr = m_d3dClearEffect->Begin(&passes, D3DXFX_DONOTSAVESTATE);
	T_ASSERT (SUCCEEDED(hr));
	if (FAILED(hr))
		return false;

	// Use our own state cache to setup render states.
	parameterCache->setRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	parameterCache->setRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	parameterCache->setRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	parameterCache->setRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
	parameterCache->setRenderState(D3DRS_STENCILENABLE, FALSE);
	parameterCache->setRenderState(D3DRS_ZENABLE, FALSE);
	parameterCache->setRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	parameterCache->setRenderState(D3DRS_ZWRITEENABLE, FALSE);

	for (int32_t i = 0; i < ntargets; ++i)
	{
		D3DXVECTOR4 clearColor;
		colors[i].storeUnaligned((float*)&clearColor);
		hr = m_d3dClearEffect->SetVector(m_d3dClearColor[i], &clearColor);
		T_ASSERT (SUCCEEDED(hr));
	}

	m_d3dClearEffect->BeginPass(0);

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

	m_d3dClearEffect->SetStateManager(0);

	hr = d3dDevice->SetViewport(&d3dvp0);
	T_ASSERT (SUCCEEDED(hr));

	return true;
}

	}
}
