#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
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

bool collectScalarParameters(
	ID3DXConstantTable* d3dConstantTable,
	std::vector< ProgramScalar >& outScalars,
	std::map< std::wstring, uint32_t >& outScalarParameterMap,
	uint32_t& outOffset
)
{
	D3DXCONSTANTTABLE_DESC dctd;
	D3DXCONSTANT_DESC dcd;
	HRESULT hr;

	hr = d3dConstantTable->GetDesc(&dctd);
	if (FAILED(hr))
		return false;

	for (UINT i = 0; i < dctd.Constants; ++i)
	{
		D3DXHANDLE handle = d3dConstantTable->GetConstant(NULL, i);
		if (!handle)
			continue;

		UINT count = 1;
		if (FAILED(d3dConstantTable->GetConstantDesc(handle, &dcd, &count)))
			continue;

		if (dcd.Class == D3DXPC_SCALAR || dcd.Class == D3DXPC_VECTOR || dcd.Class == D3DXPC_MATRIX_COLUMNS)
		{
			T_ASSERT (dcd.Type == D3DXPT_FLOAT);

			std::wstring parameterName = mbstows(dcd.Name);

			ProgramScalar scalar;
			scalar.registerIndex = dcd.RegisterIndex;
			scalar.registerCount = dcd.RegisterCount;
			scalar.offset = outOffset;
			scalar.length = dcd.Rows * dcd.Columns * dcd.Elements;

			std::map< std::wstring, uint32_t >::iterator j = outScalarParameterMap.find(parameterName);
			if (j != outScalarParameterMap.end())
				scalar.offset = j->second;
			else
			{
				outScalarParameterMap[parameterName] = outOffset;
				outOffset += std::max< uint32_t >(scalar.length, 4);
			}

			outScalars.push_back(scalar);
		}
	}

	return true;
}

bool collectSamplerParameters(
	const std::vector< std::wstring >& samplerTextures,
	std::vector< ProgramSampler >& outSamplers,
	std::map< std::wstring, uint32_t >& outTextureParameterMap,
	uint32_t& outOffset
)
{
	for (uint32_t i = 0; i < uint32_t(samplerTextures.size()); ++i)
	{
		const std::wstring& texture = samplerTextures[i];

		ProgramSampler sampler;
		sampler.stage = i;
		sampler.texture = outOffset;

		std::map< std::wstring, uint32_t >::const_iterator j = outTextureParameterMap.find(texture);
		if (j != outTextureParameterMap.end())
			sampler.texture = j->second;
		else
			outTextureParameterMap[texture] = outOffset++;

		outSamplers.push_back(sampler);
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerXbox360", 0, ProgramCompilerXbox360, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerXbox360::compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const
{
	ComRef< ID3DXConstantTable > d3dVertexConstantTable;
	ComRef< ID3DXConstantTable > d3dPixelConstantTable;

	Ref< ProgramResourceDx9 > resource = new ProgramResourceDx9();

	// Generate HLSL shaders.
	HlslProgram program;
	if (!Hlsl().generate(shaderGraph, program))
		return 0;

	// Compile shaders.
	DWORD flags = 0/*c_optimizationLevels[clamp(optimize, 0, 4)]*/;
	if (!validate)
		flags |= D3DXSHADER_SKIPVALIDATION;

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

	D3DXGetShaderConstantTable((const DWORD *)resource->m_vertexShader->GetBufferPointer(), &d3dVertexConstantTable.getAssign());
	D3DXGetShaderConstantTable((const DWORD *)resource->m_pixelShader->GetBufferPointer(), &d3dPixelConstantTable.getAssign());

	// Create scalar parameters.
	resource->m_scalarParameterDataSize = 0;

	if (!collectScalarParameters(
		d3dVertexConstantTable,
		resource->m_vertexScalars,
		resource->m_scalarParameterMap,
		resource->m_scalarParameterDataSize
	))
		return 0;

	if (!collectScalarParameters(
		d3dPixelConstantTable,
		resource->m_pixelScalars,
		resource->m_scalarParameterMap,
		resource->m_scalarParameterDataSize
	))
		return 0;

	// Create texture parameters.
	resource->m_textureParameterDataSize = 0;

	if (!collectSamplerParameters(
		program.getVertexTextures(),
		resource->m_vertexSamplers,
		resource->m_textureParameterMap,
		resource->m_textureParameterDataSize
	))
		return 0;

	if (!collectSamplerParameters(
		program.getPixelTextures(),
		resource->m_pixelSamplers,
		resource->m_textureParameterMap,
		resource->m_textureParameterDataSize
	))
		return 0;

	// Copy render state.
	resource->m_state = program.getState();

	return resource;
}

	}
}
