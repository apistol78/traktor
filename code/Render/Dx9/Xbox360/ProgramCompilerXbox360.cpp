#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Render/Dx9/Hlsl.h"
#include "Render/Dx9/HlslProgram.h"
#include "Render/Dx9/ParameterCache.h"
#include "Render/Dx9/ProgramResourceDx9.h"
#include "Render/Dx9/Xbox360/ProgramCompilerXbox360.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const DWORD c_optimizationLevels[] =
{
	D3DXSHADER_SKIPOPTIMIZATION,
	0,
	0,
	0,
	0
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
		FormatMultipleLines(log::error, hlslShader);
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
	AlignedVector< ProgramScalar >& outScalars,
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

			if (startsWith< std::wstring >(parameterName, L"__private__"))
				continue;

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

bool collectTextureParameters(
	ID3DXConstantTable* d3dConstantTable,
	const std::set< std::wstring >& textures,
	AlignedVector< ProgramTexture >& outTextures,
	std::map< std::wstring, uint32_t >& outTextureParameterMap,
	uint32_t& outOffset
)
{
	D3DXCONSTANTTABLE_DESC dctd;
	D3DXCONSTANT_DESC dcd;
	HRESULT hr;

	hr = d3dConstantTable->GetDesc(&dctd);
	if (FAILED(hr))
		return false;

	for (std::set< std::wstring >::const_iterator i = textures.begin(); i != textures.end(); ++i)
	{
		std::wstring uniformName = L"__private__" + (*i) + L"_size";

		D3DXHANDLE handle = d3dConstantTable->GetConstantByName(NULL, wstombs(uniformName).c_str());
		if (handle == NULL)
			continue;

		UINT count = 1;
		if (FAILED(d3dConstantTable->GetConstantDesc(handle, &dcd, &count)))
			continue;

		T_ASSERT (dcd.Type == D3DXPT_FLOAT);
		T_ASSERT (dcd.Class == D3DXPC_VECTOR);
		T_ASSERT (dcd.RegisterCount == 1);

		ProgramTexture pt;
		pt.texture = outOffset;
		pt.sizeIndex = dcd.RegisterIndex;

		std::map< std::wstring, uint32_t >::const_iterator j = outTextureParameterMap.find(*i);
		if (j != outTextureParameterMap.end())
			pt.texture = j->second;
		else
			outTextureParameterMap[*i] = outOffset++;

		outTextures.push_back(pt);
	}
	return true;
}

bool collectSamplerParameters(
	ID3DXConstantTable* d3dConstantTable,
	const std::map< uint32_t, std::pair< std::wstring, int32_t > >& samplers,
	AlignedVector< ProgramSampler >& outSamplers,
	std::map< std::wstring, uint32_t >& outTextureParameterMap,
	uint32_t& outOffset
)
{
	for (std::map< uint32_t, std::pair< std::wstring, int32_t > >::const_iterator i = samplers.begin(); i != samplers.end(); ++i)
	{
		ProgramSampler ps;
		ps.texture = outOffset;
		ps.stage = i->second.second;

		std::map< std::wstring, uint32_t >::const_iterator j = outTextureParameterMap.find(i->second.first);
		if (j != outTextureParameterMap.end())
			ps.texture = j->second;
		else
			outTextureParameterMap[i->second.first] = outOffset++;

		outSamplers.push_back(ps);
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerXbox360", 0, ProgramCompilerXbox360, IProgramCompiler)

const wchar_t* ProgramCompilerXbox360::getPlatformSignature() const
{
	return L"DX9";
}

Ref< ProgramResource > ProgramCompilerXbox360::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	ComRef< ID3DXConstantTable > d3dVertexConstantTable;
	ComRef< ID3DXConstantTable > d3dPixelConstantTable;

	// Generate HLSL shaders.
	HlslProgram program;
	if (!Hlsl().generate(shaderGraph, program))
	{
		log::error << L"ProgramCompilerXbox360 failed; unable to generate HLSL" << Endl;
		return 0;
	}

	// Compile shaders.
	DWORD flags = c_optimizationLevels[clamp(optimize, 0, 4)];
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
	{
		log::error << L"ProgramCompilerXbox360 failed; unable to compile vertex shader" << Endl;
		return 0;
	}

	if (!compileShader(
		program.getPixelShader(),
		"main",
		"ps_3_0",
		flags | D3DXSHADER_PARTIALPRECISION,
		resource->m_pixelShader,
		resource->m_pixelShaderHash
	))
	{
		log::error << L"ProgramCompilerXbox360 failed; unable to compile pixel shader" << Endl;
		return 0;
	}

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

	if (!collectTextureParameters(
		d3dVertexConstantTable,
		program.getVertexTextures(),
		resource->m_vertexTextures,
		resource->m_textureParameterMap,
		resource->m_textureParameterDataSize
	))
		return 0;

	if (!collectSamplerParameters(
		d3dVertexConstantTable,
		program.getVertexSamplers(),
		resource->m_vertexSamplers,
		resource->m_textureParameterMap,
		resource->m_textureParameterDataSize
	))
		return 0;

	if (resource->m_vertexSamplers.size() > ParameterCache::VertexTextureCount)
	{
		log::error << L"ProgramCompilerXbox360 failed; too many vertex samplers used (max 8)" << Endl;
		return false;
	}

	if (!collectTextureParameters(
		d3dPixelConstantTable,
		program.getPixelTextures(),
		resource->m_pixelTextures,
		resource->m_textureParameterMap,
		resource->m_textureParameterDataSize
	))
		return 0;

	if (!collectSamplerParameters(
		d3dPixelConstantTable,
		program.getPixelSamplers(),
		resource->m_pixelSamplers,
		resource->m_textureParameterMap,
		resource->m_textureParameterDataSize
	))
		return 0;

	if (resource->m_pixelSamplers.size() > ParameterCache::PixelTextureCount)
	{
		log::error << L"ProgramCompilerXbox360 failed; too many pixel samplers used (max 8)" << Endl;
		return false;
	}

	// Copy render state.
	resource->m_state = program.getState();

	// Estimate cost from number of bytes in shaders.
	if (outStats)
	{
		ComRef< ID3DXBuffer > d3dDisasmBuffer;
		HRESULT hr;

		hr = D3DXDisassembleShader((const DWORD *)resource->m_vertexShader->GetBufferPointer(), FALSE, NULL, &d3dDisasmBuffer.getAssign());
		if (SUCCEEDED(hr))
		{
			// Find and extract cost from "approximately ??? instruction slots used" comment.
			const char* tmp = (const char *)d3dDisasmBuffer->GetBufferPointer();
			const char* costp = strstr(tmp, "approximately");
			if (costp)
				outStats->vertexCost = atoi(costp + 14);
		}

		hr = D3DXDisassembleShader((const DWORD *)resource->m_pixelShader->GetBufferPointer(), FALSE, NULL, &d3dDisasmBuffer.getAssign());
		if (SUCCEEDED(hr))
		{
			// Find and extract cost from "approximately ??? instruction slots used" comment.
			const char* tmp = (const char *)d3dDisasmBuffer->GetBufferPointer();
			const char* costp = strstr(tmp, "approximately");
			if (costp)
				outStats->pixelCost = atoi(costp + 14);
		}
	}

	return resource;
}

bool ProgramCompilerXbox360::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	std::wstring& outShader
) const
{
	HlslProgram hlslProgram;
	if (!Hlsl().generate(shaderGraph, hlslProgram))
		return false;

	outShader =
		std::wstring(L"// Vertex shader\n") +
		std::wstring(L"\n") +
		hlslProgram.getVertexShader() +
		std::wstring(L"\n") +
		std::wstring(L"// Pixel shader\n") +
		std::wstring(L"\n") +
		hlslProgram.getPixelShader();

	return true;
}

	}
}
