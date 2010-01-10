#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Render/Dx9/Hlsl.h"
#include "Render/Dx9/HlslProgram.h"
#include "Render/Dx9/ProgramResourceDx9.h"
#include "Render/Dx9/Win32/ProgramCompilerWin32.h"
#include "Render/Dx9/Win32/ProgramWin32.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const DWORD c_optimizationLevels[] =
{
	D3DXSHADER_SKIPOPTIMIZATION,
	D3DXSHADER_OPTIMIZATION_LEVEL0,
	D3DXSHADER_OPTIMIZATION_LEVEL1,
	D3DXSHADER_OPTIMIZATION_LEVEL2,
	D3DXSHADER_OPTIMIZATION_LEVEL3
};

bool compileShader(
	const std::wstring& hlslShader,
	const std::string& entry,
	const std::string& profile,
	DWORD flags,
	ComRef< ID3DXBuffer >& outProgramResource,
	uint32_t& outProgramHash
)
{
	ComRef< ID3DXBuffer > d3dErrorMsgs;
	HRESULT hr;

	hr = D3DXCompileShader(
		wstombs(hlslShader).c_str(),
		(UINT)hlslShader.length(),
		NULL,
		NULL,
		entry.c_str(),
		profile.c_str(),
		flags,
		&outProgramResource.getAssign(),
		&d3dErrorMsgs.getAssign(),
		NULL
	);
	if (FAILED(hr))
	{
		if (d3dErrorMsgs)
			log::error << L"HLSL compile error : \"" << trim(mbstows((LPCSTR)d3dErrorMsgs->GetBufferPointer())) << L"\"" << Endl;
		log::error << hlslShader << Endl;
		return false;
	}

	Adler32 hash;
	hash.begin();
	hash.feed(outProgramResource->GetBufferPointer(), outProgramResource->GetBufferSize());
	hash.end();

	outProgramHash = hash.get();

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerWin32", 0, ProgramCompilerWin32, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerWin32::compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const
{
	HlslProgram program;
	if (!Hlsl().generate(shaderGraph, program))
		return 0;

	DWORD flags = c_optimizationLevels[max(min(optimize, 4), 0)];
	if (!validate)
		flags |= D3DXSHADER_SKIPVALIDATION;

	Ref< ProgramResourceDx9 > resource = new ProgramResourceDx9();

	if (!compileShader(
		program.getVertexShader(),
		"main",
		"vs_3_0",
		flags,
		resource->m_vertexShader,
		resource->m_vertexShaderHash
	))
		return 0;

	if (!compileShader(
		program.getPixelShader(),
		"main",
		"ps_3_0",
		flags,
		resource->m_pixelShader,
		resource->m_pixelShaderHash
	))
		return 0;

	resource->m_state = program.getState();
	return resource;
}

	}
}
