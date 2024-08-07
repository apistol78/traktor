/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/ComRef.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Render/Dx11/Blob.h"
#include "Render/Dx11/ProgramResourceDx11.h"
#include "Render/Dx11/Editor/ProgramCompilerDx11.h"
#include "Render/Dx11/Editor/Hlsl/Hlsl.h"
#include "Render/Dx11/Editor/Hlsl/HlslProgram.h"

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

HMODULE s_hLibraryCompiler = NULL;
HMODULE s_hLibraryReflector = NULL;

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

HRESULT (WINAPI *s_D3DReflect)(
	LPCVOID pSrcData,
	SIZE_T SrcDataSize,
	REFIID pInterface,
	void** ppReflector
) = NULL;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerDx11", 0, ProgramCompilerDx11, IProgramCompiler)

ProgramCompilerDx11::ProgramCompilerDx11()
{
	if (s_hLibraryCompiler == NULL)
	{
		s_hLibraryCompiler = LoadLibrary(L"d3dcompiler_46.dll");
		if (s_hLibraryCompiler != NULL)
		{
			(FARPROC&)s_D3DCompile2 = GetProcAddress(s_hLibraryCompiler, "D3DCompile2");
			if (!s_D3DCompile2)
			{
				log::error << L"Failed to create HLSL compiler; Symbol \"D3DCompile2\" not found" << Endl;
				FreeLibrary(s_hLibraryCompiler); s_hLibraryCompiler = NULL;
			}
		}
	}

	if (s_hLibraryReflector == NULL)
	{
		s_hLibraryReflector = LoadLibrary(L"d3dcompiler_46.dll");
		if (s_hLibraryReflector != NULL)
		{
			(FARPROC&)s_D3DReflect = GetProcAddress(s_hLibraryReflector, "D3DReflect");
			if (!s_D3DReflect)
			{
				log::error << L"Failed to create HLSL compiler; Symbol \"D3DReflect\" not found" << Endl;
				FreeLibrary(s_hLibraryReflector); s_hLibraryReflector = NULL;
			}
		}
	}
}

const wchar_t* ProgramCompilerDx11::getRendererSignature() const
{
	return L"DX11";
}

Ref< ProgramResource > ProgramCompilerDx11::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name
) const
{
	if (!s_D3DCompile2 || !s_D3DReflect)
	{
		log::error << L"Cannot compile due to missing D3D compiler symbols." << Endl;
		return nullptr;
	}

	HlslProgram hlslProgram;
	if (!Hlsl().generate(shaderGraph, hlslProgram))
		return nullptr;

	const int32_t maxOptimize = sizeof_array(c_optimizationFlags) - 1;
	const int32_t optimize = (settings != nullptr ? settings->getProperty< int32_t >(L"Hlsl.Optimize", maxOptimize) : maxOptimize);

	ComRef< ID3DBlob > d3dVertexShader;
	ComRef< ID3DBlob > d3dPixelShader;
	ComRef< ID3DBlob > d3dErrorMsgs;
	HRESULT hr;

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
		&d3dVertexShader.getAssign(),
		&d3dErrorMsgs.getAssign()
	);
	if (FAILED(hr) || hr == S_FALSE)
	{
		log::error << L"Failed to compile vertex shader, hr = " << int32_t(hr) << Endl;
		if (d3dErrorMsgs)
			log::error << mbstows((LPCSTR)d3dErrorMsgs->GetBufferPointer()) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, hlslProgram.getVertexShader());
		return nullptr;
	}

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
		&d3dPixelShader.getAssign(),
		&d3dErrorMsgs.getAssign()
	);
	if (FAILED(hr) || hr == S_FALSE)
	{
		log::error << L"Failed to compile pixel shader, hr = " << int32_t(hr) << Endl;
		if (d3dErrorMsgs)
			log::error << mbstows((LPCSTR)d3dErrorMsgs->GetBufferPointer()) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, hlslProgram.getPixelShader());
		return nullptr;
	}

	// Create our program resource container.
	Ref< ProgramResourceDx11 > resource = new ProgramResourceDx11();
	resource->m_vertexShader = Blob::create(d3dVertexShader->GetBufferPointer(), d3dVertexShader->GetBufferSize());
	resource->m_pixelShader = Blob::create(d3dPixelShader->GetBufferPointer(), d3dPixelShader->GetBufferSize());

	std::map< std::wstring, ProgramResourceDx11::ParameterDesc > parameterMap;
	uint32_t parameterScalarOffset = 0;
	uint32_t parameterTextureOffset = 0;
	uint32_t parameterStructBufferOffset = 0;

	// Extract parameters and samplers from vertex shader.
	{
		ComRef< ID3D11ShaderReflection > d3dShaderReflection;
		ID3D11ShaderReflectionConstantBuffer* d3dConstantBufferReflection;
		ID3D11ShaderReflectionVariable* d3dVariableReflection;
		ID3D11ShaderReflectionType* d3dTypeReflection;
		D3D11_SHADER_DESC dsd;
		D3D11_SHADER_BUFFER_DESC dsbd;
		D3D11_SHADER_VARIABLE_DESC dsvd;
		D3D11_SHADER_TYPE_DESC dstd;
		D3D11_SHADER_INPUT_BIND_DESC dsibd;

		hr = (*s_D3DReflect)(
			d3dVertexShader->GetBufferPointer(),
			d3dVertexShader->GetBufferSize(),
			IID_ID3D11ShaderReflection,
			(void**)&d3dShaderReflection.getAssign()
		);
		if (FAILED(hr))
		{
			log::error << L"Failed to reflect vertex shader, hr = " << int32_t(hr) << Endl;
			return nullptr;
		}

		d3dShaderReflection->GetDesc(&dsd);

		// Scalar parameters.
		for (UINT i = 0; i < dsd.ConstantBuffers; ++i)
		{
			d3dConstantBufferReflection = d3dShaderReflection->GetConstantBufferByIndex(i);
			T_ASSERT(d3dConstantBufferReflection);

			d3dConstantBufferReflection->GetDesc(&dsbd);
			if (dsbd.Type != D3D_CT_CBUFFER || dsbd.Size == 0)
				continue;

			ProgramResourceDx11::CBufferDesc* vertexCBuffer = nullptr;
			for (uint32_t j = 0; j < sizeof_array(resource->m_vertexCBuffers); ++j)
			{
				if (resource->m_vertexCBuffers[j].size == 0)
				{
					vertexCBuffer = &resource->m_vertexCBuffers[j];
					break;
				}
			}
			T_FATAL_ASSERT (vertexCBuffer != nullptr);

			T_ASSERT((dsbd.Size & 3) == 0);
			vertexCBuffer->size = dsbd.Size;

			for (UINT j = 0; j < dsbd.Variables; ++j)
			{
				d3dVariableReflection = d3dConstantBufferReflection->GetVariableByIndex(j);
				T_ASSERT(d3dVariableReflection);

				d3dTypeReflection = d3dVariableReflection->GetType();
				T_ASSERT(d3dTypeReflection);

				d3dVariableReflection->GetDesc(&dsvd);
				T_ASSERT((dsvd.StartOffset & 3) == 0);

				d3dTypeReflection->GetDesc(&dstd);
				if (dstd.Class == D3D_SVC_STRUCT)
					continue;
				T_ASSERT(dstd.Type == D3D10_SVT_FLOAT);

				std::wstring name = mbstows(dsvd.Name);

				std::map< std::wstring, ProgramResourceDx11::ParameterDesc >::iterator it = parameterMap.find(name);
				if (it == parameterMap.end())
				{
					uint32_t parameterOffset = alignUp(parameterScalarOffset, 4);
					uint32_t parameterCount = dsvd.Size >> 2;

					vertexCBuffer->parameters.push_back(ProgramResourceDx11::ParameterMappingDesc(
						dsvd.StartOffset,
						parameterOffset,
						parameterCount
					));

					ProgramResourceDx11::ParameterDesc& pd = parameterMap[name];
					pd.name = name;
					pd.offset = parameterOffset;
					pd.count = parameterCount;

					parameterScalarOffset = parameterOffset + parameterCount;
				}
				else
				{
					if (it->second.count != (dsvd.Size >> 2))
					{
						log::error << L"Mismatching parameter size \"" << name << L"\" in vertex shader." << Endl;
						return nullptr;
					}

					vertexCBuffer->parameters.push_back(ProgramResourceDx11::ParameterMappingDesc(
						dsvd.StartOffset,
						it->second.offset,
						it->second.count
					));
				}
			}
		}

		// Texture/sampler parameters.
		for (UINT i = 0; i < dsd.BoundResources; ++i)
		{
			d3dShaderReflection->GetResourceBindingDesc(i, &dsibd);
			if (dsibd.Type == D3D10_SIT_TEXTURE)
			{
				T_ASSERT(dsibd.BindCount == 1);

				std::wstring name = mbstows(dsibd.Name);

				auto it = parameterMap.find(name);
				if (it == parameterMap.end())
				{
					uint32_t resourceIndex = parameterTextureOffset;

					ProgramResourceDx11::ParameterDesc& pd = parameterMap[name];
					pd.name = name;
					pd.offset = resourceIndex;
					pd.count = 0;

					resource->m_vertexTextureBindings.push_back(ProgramResourceDx11::ResourceBindingDesc(
						dsibd.BindPoint,
						resourceIndex
					));

					++parameterTextureOffset;
				}
				else
				{
					uint32_t resourceIndex = it->second.offset;

					resource->m_vertexTextureBindings.push_back(ProgramResourceDx11::ResourceBindingDesc(
						dsibd.BindPoint,
						resourceIndex
					));
				}
			}
			else if (dsibd.Type == D3D11_SIT_STRUCTURED)
			{
				T_ASSERT(dsibd.BindCount == 1);

				std::wstring name = mbstows(dsibd.Name);

				auto it = parameterMap.find(name);
				if (it == parameterMap.end())
				{
					uint32_t resourceIndex = parameterStructBufferOffset;

					ProgramResourceDx11::ParameterDesc& pd = parameterMap[name];
					pd.name = name;
					pd.offset = resourceIndex;
					pd.count = 0;

					resource->m_vertexStructBufferBindings.push_back(ProgramResourceDx11::ResourceBindingDesc(
						dsibd.BindPoint,
						resourceIndex
					));

					++parameterStructBufferOffset;
				}
				else
				{
					uint32_t resourceIndex = it->second.offset;

					resource->m_vertexStructBufferBindings.push_back(ProgramResourceDx11::ResourceBindingDesc(
						dsibd.BindPoint,
						resourceIndex
					));
				}
			}
			else if (dsibd.Type == D3D10_SIT_SAMPLER)
			{
				std::wstring name = mbstows(dsibd.Name);

				const auto& samplers = hlslProgram.getD3DVertexSamplers();
				auto it = samplers.find(name);
				if (it == samplers.end())
				{
					log::error << L"Unknown sampler \"" << name << L"\" referenced in vertex." << Endl;
					return nullptr;
				}

				resource->m_vertexSamplers.push_back(it->second);
			}
		}
	}

	// Extract parameters and samplers from pixel shader.
	{
		ComRef< ID3D11ShaderReflection > d3dShaderReflection;
		ID3D11ShaderReflectionConstantBuffer* d3dConstantBufferReflection;
		ID3D11ShaderReflectionVariable* d3dVariableReflection;
		ID3D11ShaderReflectionType* d3dTypeReflection;
		D3D11_SHADER_DESC dsd;
		D3D11_SHADER_BUFFER_DESC dsbd;
		D3D11_SHADER_VARIABLE_DESC dsvd;
		D3D11_SHADER_TYPE_DESC dstd;
		D3D11_SHADER_INPUT_BIND_DESC dsibd;

		hr = (*s_D3DReflect)(
			d3dPixelShader->GetBufferPointer(),
			d3dPixelShader->GetBufferSize(),
			IID_ID3D11ShaderReflection,
			(void**)&d3dShaderReflection.getAssign()
		);
		if (FAILED(hr))
		{
			log::error << L"Failed to reflect pixel shader, hr = " << int32_t(hr) << Endl;
			return nullptr;
		}

		d3dShaderReflection->GetDesc(&dsd);

		// Scalar parameters.
		for (UINT i = 0; i < dsd.ConstantBuffers; ++i)
		{
			d3dConstantBufferReflection = d3dShaderReflection->GetConstantBufferByIndex(i);
			T_ASSERT(d3dConstantBufferReflection);

			d3dConstantBufferReflection->GetDesc(&dsbd);
			if (dsbd.Type != D3D_CT_CBUFFER || dsbd.Size == 0)
				continue;

			ProgramResourceDx11::CBufferDesc* pixelCBuffer = nullptr;
			for (uint32_t j = 0; j < sizeof_array(resource->m_pixelCBuffers); ++j)
			{
				if (resource->m_pixelCBuffers[j].size == 0)
				{
					pixelCBuffer = &resource->m_pixelCBuffers[j];
					break;
				}
			}
			T_FATAL_ASSERT (pixelCBuffer != nullptr);

			T_ASSERT((dsbd.Size & 3) == 0);
			pixelCBuffer->size = dsbd.Size;

			for (UINT j = 0; j < dsbd.Variables; ++j)
			{
				d3dVariableReflection = d3dConstantBufferReflection->GetVariableByIndex(j);
				T_ASSERT(d3dVariableReflection);

				d3dTypeReflection = d3dVariableReflection->GetType();
				T_ASSERT(d3dTypeReflection);

				d3dVariableReflection->GetDesc(&dsvd);
				T_ASSERT((dsvd.StartOffset & 3) == 0);

				d3dTypeReflection->GetDesc(&dstd);
				if (dstd.Class == D3D_SVC_STRUCT)
					continue;
				T_ASSERT(dstd.Type == D3D10_SVT_FLOAT);

				std::wstring name = mbstows(dsvd.Name);

				std::map< std::wstring, ProgramResourceDx11::ParameterDesc >::iterator it = parameterMap.find(name);
				if (it == parameterMap.end())
				{
					uint32_t parameterOffset = alignUp(parameterScalarOffset, 4);
					uint32_t parameterCount = dsvd.Size >> 2;

					pixelCBuffer->parameters.push_back(ProgramResourceDx11::ParameterMappingDesc(
						dsvd.StartOffset,
						parameterOffset,
						parameterCount
					));

					ProgramResourceDx11::ParameterDesc& pd = parameterMap[name];
					pd.name = name;
					pd.offset = parameterOffset;
					pd.count = parameterCount;

					parameterScalarOffset = parameterOffset + parameterCount;
				}
				else
				{
					if (it->second.count != (dsvd.Size >> 2))
					{
						log::error << L"Mismatching parameter size \"" << name << L"\" in pixel shader." << Endl;
						return nullptr;
					}

					pixelCBuffer->parameters.push_back(ProgramResourceDx11::ParameterMappingDesc(
						dsvd.StartOffset,
						it->second.offset,
						it->second.count
					));
				}
			}
		}

		// Texture/sampler parameters.
		for (UINT i = 0; i < dsd.BoundResources; ++i)
		{
			d3dShaderReflection->GetResourceBindingDesc(i, &dsibd);
			if (dsibd.Type == D3D10_SIT_TEXTURE)
			{
				T_ASSERT(dsibd.BindCount == 1);

				std::wstring name = mbstows(dsibd.Name);

				auto it = parameterMap.find(name);
				if (it == parameterMap.end())
				{
					uint32_t resourceIndex = parameterTextureOffset;

					ProgramResourceDx11::ParameterDesc& pd = parameterMap[name];
					pd.name = name;
					pd.offset = resourceIndex;
					pd.count = 0;

					resource->m_pixelTextureBindings.push_back(ProgramResourceDx11::ResourceBindingDesc(
						dsibd.BindPoint,
						resourceIndex
					));

					++parameterTextureOffset;
				}
				else
				{
					uint32_t resourceIndex = it->second.offset;

					resource->m_pixelTextureBindings.push_back(ProgramResourceDx11::ResourceBindingDesc(
						dsibd.BindPoint,
						resourceIndex
					));
				}
			}
			else if (dsibd.Type == D3D11_SIT_STRUCTURED)
			{
				T_ASSERT(dsibd.BindCount == 1);

				std::wstring name = mbstows(dsibd.Name);

				auto it = parameterMap.find(name);
				if (it == parameterMap.end())
				{
					uint32_t resourceIndex = parameterStructBufferOffset;

					ProgramResourceDx11::ParameterDesc& pd = parameterMap[name];
					pd.name = name;
					pd.offset = resourceIndex;
					pd.count = 0;

					resource->m_pixelStructBufferBindings.push_back(ProgramResourceDx11::ResourceBindingDesc(
						dsibd.BindPoint,
						resourceIndex
					));

					++parameterStructBufferOffset;
				}
				else
				{
					uint32_t resourceIndex = it->second.offset;

					resource->m_pixelStructBufferBindings.push_back(ProgramResourceDx11::ResourceBindingDesc(
						dsibd.BindPoint,
						resourceIndex
					));
				}
			}
			else if (dsibd.Type == D3D10_SIT_SAMPLER)
			{
				std::wstring name = mbstows(dsibd.Name);

				const auto& samplers = hlslProgram.getD3DPixelSamplers();
				auto it = samplers.find(name);
				if (it == samplers.end())
				{
					log::error << L"Unknown sampler \"" << name << L"\" referenced in pixel shader." << Endl;
					return nullptr;
				}

				resource->m_pixelSamplers.push_back(it->second);
			}
		}
	}

	// Insert parameters into resource.
	for (const auto& pm : parameterMap)
		resource->m_parameters.push_back(pm.second);

	resource->m_parameterScalarSize = parameterScalarOffset;
	resource->m_parameterTextureSize = parameterTextureOffset;
	resource->m_parameterStructBufferSize = parameterStructBufferOffset;

	// Calculate vertex shader hash.
	{
		Murmur3 hash;
		hash.begin();
		hash.feed(d3dVertexShader->GetBufferPointer(), d3dVertexShader->GetBufferSize());
		hash.end();
		resource->m_vertexShaderHash = hash.get();
	}

	// Calculate pixel shader hash.
	{
		Murmur3 hash;
		hash.begin();
		hash.feed(d3dPixelShader->GetBufferPointer(), d3dPixelShader->GetBufferSize());
		hash.end();
		resource->m_pixelShaderHash = hash.get();
	}

	resource->m_d3dRasterizerDesc = hlslProgram.getD3DRasterizerDesc();
	resource->m_d3dDepthStencilDesc = hlslProgram.getD3DDepthStencilDesc();
	resource->m_d3dBlendDesc = hlslProgram.getD3DBlendDesc();
	resource->m_stencilReference = hlslProgram.getStencilReference();
	return resource;
}

bool ProgramCompilerDx11::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	const std::wstring& name,
	std::wstring& outVertexShader,
	std::wstring& outPixelShader,
	std::wstring& outComputeShader
) const
{
	HlslProgram hlslProgram;
	if (!Hlsl().generate(shaderGraph, hlslProgram))
		return false;

	outVertexShader = hlslProgram.getVertexShader();
	outPixelShader = hlslProgram.getPixelShader();
	outComputeShader = hlslProgram.getComputeShader();
	return true;
}

	}
}
