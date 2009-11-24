#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/String.h"
#include "Render/Dx9/Hlsl.h"
#include "Render/Dx9/HlslProgram.h"
#include "Render/Dx9/ProgramResourceDx9.h"
#include "Render/Dx9/Xbox360/ProgramCompilerXbox360.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

bool compileShader(
	const std::wstring& hlslShader,
	const std::string& entry,
	const std::string& profile,
	DWORD flags,
	ComRef< ID3DXBuffer >& outProgramResource
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

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerXbox360", 0, ProgramCompilerXbox360, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerXbox360::compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const
{
	HlslProgram program;
	if (!Hlsl().generate(shaderGraph, program))
		return 0;

	Ref< ProgramResource > programResource = compile(program, optimize, validate);
	if (!programResource)
		return 0;

	return programResource;
}

Ref< ProgramResource > ProgramCompilerXbox360::compile(const HlslProgram& hlslProgram, int32_t optimize, bool validate) const
{
	const DWORD c_optimizationLevels[] =
	{
		D3DXSHADER_SKIPOPTIMIZATION,
		0,
		0,
		0,
		0
	};

	DWORD flags = c_optimizationLevels[max(min(optimize, 4), 0)];
	if (!validate)
		flags |= D3DXSHADER_SKIPVALIDATION;

	Ref< ProgramResourceDx9 > resource = new ProgramResourceDx9();

	if (!compileShader(
		hlslProgram.getVertexShader(),
		"main",
		"vs_3_0",
		flags,
		resource->m_vertexShader
	))
		return 0;

	if (!compileShader(
		hlslProgram.getPixelShader(),
		"main",
		"ps_3_0",
		flags,
		resource->m_pixelShader
	))
		return 0;

	resource->m_state = hlslProgram.getState();
	return resource;
}

	}
}
