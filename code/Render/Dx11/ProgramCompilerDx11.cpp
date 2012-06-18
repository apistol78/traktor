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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerDx11", 0, ProgramCompilerDx11, IProgramCompiler)

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

	hr = D3DX11CompileFromMemory(
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
	if (FAILED(hr))
	{
		log::error << L"Failed to compile vertex shader, hr = " << int32_t(hr) << Endl;
		if (d3dErrorMsgs)
			log::error << mbstows((LPCSTR)d3dErrorMsgs->GetBufferPointer()) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, hlslProgram.getVertexShader());
		return 0;
	}

	hr = D3DX11CompileFromMemory(
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
	if (FAILED(hr))
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
