#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/TString.h"
#include "Render/Dx11/Hlsl.h"
#include "Render/Dx11/HlslProgram.h"
#include "Render/Dx11/ProgramCompilerDx11.h"
#include "Render/Dx11/ProgramResourceDx11.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

uint32_t c_optimizationFlags[] =
{
	D3D10_SHADER_SKIP_OPTIMIZATION,
	D3D10_SHADER_OPTIMIZATION_LEVEL0,
	D3D10_SHADER_OPTIMIZATION_LEVEL1,
	D3D10_SHADER_OPTIMIZATION_LEVEL2,
	D3D10_SHADER_OPTIMIZATION_LEVEL3
};

HMODULE s_hLibrary = NULL;

HRESULT (WINAPI *s_D3DCompile2)(
	LPCVOID pSrcData,
	SIZE_T SrcDataSize,
	LPCSTR pSourceName,
	CONST D3D_SHADER_MACRO* pDefines,
	ID3DInclude* pInclude,
	LPCSTR pEntrypoint,
	LPCSTR pTarget,
	UINT Flags1,
	UINT Flags2,
	UINT SecondaryDataFlags,
	LPCVOID pSecondaryData,
	SIZE_T SecondaryDataSize,
	ID3DBlob** ppCode,
	ID3DBlob** ppErrorMsgs
) = NULL;

HRESULT (WINAPI *s_D3DX11CompileFromMemory)(
	LPCSTR pSrcData,
	SIZE_T SrcDataLen,
	LPCSTR pFileName,
	CONST D3D10_SHADER_MACRO* pDefines,
	LPD3D10INCLUDE pInclude,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	UINT Flags1,
	UINT Flags2,
	ID3DX11ThreadPump* pPump,
	ID3D10Blob** ppShader,
	ID3D10Blob** ppErrorMsgs,
	HRESULT* pHResult
) = NULL;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerDx11", 0, ProgramCompilerDx11, IProgramCompiler)

ProgramCompilerDx11::ProgramCompilerDx11()
{
	if (s_hLibrary == NULL)
	{
		s_hLibrary = LoadLibrary(L"d3dcompiler_46.dll");
		if (s_hLibrary != NULL)
		{
			(FARPROC&)s_D3DCompile2 = GetProcAddress(s_hLibrary, "D3DCompile2");
			if (!s_D3DCompile2)
			{
				log::error << L"Failed to create new HLSL compiler; Symbol \"D3DCompile2\" not found" << Endl;
				FreeLibrary(s_hLibrary); s_hLibrary = NULL;
			}
		}
	}

	if (s_hLibrary == NULL)
	{
		s_hLibrary = LoadLibrary(L"d3dx11_43.dll");
		if (s_hLibrary != NULL)
		{
			(FARPROC&)s_D3DX11CompileFromMemory = GetProcAddress(s_hLibrary, "D3DX11CompileFromMemory");
			if (!s_D3DX11CompileFromMemory)
			{
				log::error << L"Failed to create old HLSL compiler; Symbol \"D3DX11CompileFromMemory\" not found" << Endl;
				FreeLibrary(s_hLibrary); s_hLibrary = NULL;
			}
			else
				log::warning << L"Using old HLSL compiler; Please reinstall DirectX redist" << Endl;
		}
	}
}

const wchar_t* ProgramCompilerDx11::getPlatformSignature() const
{
	return L"DX11";
}

Ref< ProgramResource > ProgramCompilerDx11::compile(
	const ShaderGraph* shaderGraph,
	int32_t optimize,
	bool validate,
	IProgramHints* hints,
	Stats* outStats
) const
{
	HlslProgram hlslProgram;
	if (!Hlsl().generate(shaderGraph, hints, hlslProgram))
		return 0;

	optimize = clamp< int32_t >(optimize, 0, sizeof_array(c_optimizationFlags));

	Ref< ProgramResourceDx11 > resource = new ProgramResourceDx11();

	ComRef< ID3DBlob > d3dErrorMsgs;
	HRESULT hr;

	hr = S_FALSE;
	if (s_D3DCompile2)
	{
		hr = (*s_D3DCompile2)(
			wstombs(hlslProgram.getVertexShader()).c_str(),
			hlslProgram.getVertexShader().length(),
			"generated.vs",
			NULL,
			NULL,
			"main",
			"vs_4_0",
			c_optimizationFlags[optimize],
			0,
			0,
			NULL,
			0,
			&resource->m_vertexShader.getAssign(),
			&d3dErrorMsgs.getAssign()
		);
	}
	else if (s_D3DX11CompileFromMemory)
	{
		hr = (*s_D3DX11CompileFromMemory)(
			wstombs(hlslProgram.getVertexShader()).c_str(),
			hlslProgram.getVertexShader().length(),
			"generated.vs",
			NULL,
			NULL,
			"main",
			"vs_4_0",
			c_optimizationFlags[optimize],
			0,
			NULL,
			&resource->m_vertexShader.getAssign(),
			&d3dErrorMsgs.getAssign(),
			NULL
		);
	}
	if (FAILED(hr) || hr == S_FALSE)
	{
		log::error << L"Failed to compile vertex shader, hr = " << int32_t(hr) << Endl;
		if (d3dErrorMsgs)
			log::error << mbstows((LPCSTR)d3dErrorMsgs->GetBufferPointer()) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, hlslProgram.getVertexShader());
		return 0;
	}

	hr = S_FALSE;
	if (s_D3DCompile2)
	{
		hr = (*s_D3DCompile2)(
			wstombs(hlslProgram.getPixelShader()).c_str(),
			hlslProgram.getPixelShader().length(),
			"generated.ps",
			NULL,
			NULL,
			"main",
			"ps_4_0",
			c_optimizationFlags[optimize],
			0,
			0,
			NULL,
			0,
			&resource->m_pixelShader.getAssign(),
			&d3dErrorMsgs.getAssign()
		);
	}
	else if (s_D3DX11CompileFromMemory)
	{
		hr = (*s_D3DX11CompileFromMemory)(
			wstombs(hlslProgram.getPixelShader()).c_str(),
			hlslProgram.getPixelShader().length(),
			"generated.ps",
			NULL,
			NULL,
			"main",
			"ps_4_0",
			c_optimizationFlags[optimize],
			0,
			NULL,
			&resource->m_pixelShader.getAssign(),
			&d3dErrorMsgs.getAssign(),
			NULL
		);
	}
	if (FAILED(hr) || hr == S_FALSE)
	{
		log::error << L"Failed to compile pixel shader, hr = " << int32_t(hr) << Endl;
		if (d3dErrorMsgs)
			log::error << mbstows((LPCSTR)d3dErrorMsgs->GetBufferPointer()) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, hlslProgram.getPixelShader());
		return 0;
	}

	{
		Adler32 hash;
		hash.begin();
		hash.feed(resource->m_vertexShader->GetBufferPointer(), resource->m_vertexShader->GetBufferSize());
		hash.end();
		resource->m_vertexShaderHash = hash.get();
	}

	{
		Adler32 hash;
		hash.begin();
		hash.feed(resource->m_pixelShader->GetBufferPointer(), resource->m_pixelShader->GetBufferSize());
		hash.end();
		resource->m_pixelShaderHash = hash.get();
	}

	resource->m_d3dRasterizerDesc = hlslProgram.getD3DRasterizerDesc();
	resource->m_d3dDepthStencilDesc = hlslProgram.getD3DDepthStencilDesc();
	resource->m_d3dBlendDesc = hlslProgram.getD3DBlendDesc();
	resource->m_stencilReference = hlslProgram.getStencilReference();
	resource->m_d3dVertexSamplers = hlslProgram.getD3DVertexSamplers();
	resource->m_d3dPixelSamplers = hlslProgram.getD3DPixelSamplers();

	return resource;
}

	}
}
