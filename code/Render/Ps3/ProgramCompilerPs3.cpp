/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if !defined(_PS3)

#include "Render/Ps3/PlatformPs3.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Ps3/ProgramResourcePs3.h"
#include "Render/Ps3/ProgramCompilerPs3.h"
#include "Render/Ps3/Cg/Cg.h"
#include "Render/Ps3/Cg/CgProgram.h"
#include "Render/Ps3/Cg/CgType.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

bool collectScalarParameters(
	const ShaderGraph* shaderGraph,
	CGCbin* shaderBin,
	bool isFragmentProfile,
	std::vector< ProgramScalar >& outScalars,
	std::map< std::wstring, ScalarParameter >& outScalarParameterMap,
	uint32_t& outOffset
)
{
	size_t shaderBinSize = sceCgcGetBinSize(shaderBin);
	if (!shaderBinSize)
	{
		log::error << L"Invalid shader image" << Endl;
		return false;
	}

	// Create a copy of program; we don't want to modify source bin.
	AutoArrayPtr< uint8_t > programNV(new uint8_t [shaderBinSize]);
	std::memcpy(programNV.ptr(), sceCgcGetBinData(shaderBin), shaderBinSize);

	CGprogram program = (CGprogram)programNV.ptr();
	cellGcmCgInitProgram(program);

	const RefArray< Node >& nodes = shaderGraph->getNodes();
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		std::wstring parameterName;
		int32_t parameterCount;
		int32_t parameterSize = -1;

		if (const Uniform* uniformNode = dynamic_type_cast< const Uniform* >(*i))
		{
			parameterName = uniformNode->getParameterName();
			parameterCount = 1;
			switch (uniformNode->getParameterType())
			{
			case PtScalar:
				parameterSize = 1;
				break;

			case PtVector:
				parameterSize = 4;
				break;

			case PtMatrix:
				parameterSize = 16;
				break;
			}
		}
		else if (const IndexedUniform* indexedUniformNode = dynamic_type_cast< const IndexedUniform* >(*i))
		{
			parameterName = indexedUniformNode->getParameterName();
			parameterCount = indexedUniformNode->getLength();
			switch (indexedUniformNode->getParameterType())
			{
			case PtScalar:
				parameterSize = 1;
				break;

			case PtVector:
				parameterSize = 4;
				break;

			case PtMatrix:
				parameterSize = 16;
				break;
			}
		}
		else if (const Instance* instanceName = dynamic_type_cast< const Instance* >(*i))
		{
			parameterName = L"__private__instanceID";
			parameterCount = 1;
			parameterSize = 1;
		}

		if (parameterSize > 0)
		{
			uint32_t quadCount = ((parameterSize + 3) / 4) * parameterCount;

			ProgramScalar scalar;
			scalar.vertexRegisterIndex = 0;
			scalar.vertexRegisterCount = 0;
			scalar.offset = outOffset;

			bool scalarUsed = false;

			if (!isFragmentProfile)
			{
				std::wstring indexedParameterName = parameterName;
				if (quadCount > 1)
					indexedParameterName = parameterName + L"[0]";

				CGparameter parameter = cellGcmCgGetNamedParameter(program, wstombs(indexedParameterName).c_str());
				if (parameter)
				{
					uint32_t resourceIndex = cellGcmCgGetParameterResourceIndex(program, parameter);
					if (resourceIndex != ~0UL)
					{
						scalar.vertexRegisterIndex = resourceIndex;
						scalar.vertexRegisterCount = quadCount;
						scalarUsed = true;
					}
				}
			}
			else
			{
				for (uint32_t j = 0; j < quadCount; ++j)
				{
					std::wstring indexedParameterName = parameterName;
					if (quadCount > 1)
						indexedParameterName = parameterName + L"[" + toString(j) + L"]";

					CGparameter parameter = cellGcmCgGetNamedParameter(program, wstombs(indexedParameterName).c_str());
					if (parameter)
					{
						uint32_t constantCount = cellGcmCgGetEmbeddedConstantCount(program, parameter);
						if (constantCount > 0)
						{
							for (uint32_t k = 0; k < constantCount; ++k)
							{
								uint32_t constantOffset = cellGcmCgGetEmbeddedConstantOffset(program, parameter, k);
								FragmentOffset fragmentOffset;
								fragmentOffset.ucodeOffset = constantOffset;
								fragmentOffset.parameterOffset = j * 4;
								scalar.fragmentOffsets.push_back(fragmentOffset);
							}

							scalarUsed = true;
						}
					}
				}
			}

			if (scalarUsed)
			{
				std::map< std::wstring, ScalarParameter >::iterator j = outScalarParameterMap.find(parameterName);
				if (j != outScalarParameterMap.end())
				{
					scalar.offset = j->second.offset;
					j->second.usage |= isFragmentProfile ? SuPixel : SuVertex;
				}
				else
				{
					outScalarParameterMap[parameterName].offset = outOffset;
					outScalarParameterMap[parameterName].usage = isFragmentProfile ? SuPixel : SuVertex;
					outOffset += quadCount * 4;
				}

				outScalars.push_back(scalar);
			}
		}
	}

	return true;
}

bool collectSamplerParameters(
	const std::map< std::wstring, int32_t >& samplerTextures,
	std::vector< ProgramSampler >& outSamplers,
	std::map< std::wstring, uint32_t >& outTextureParameterMap,
	uint32_t& outOffset
)
{
	for (std::map< std::wstring, int32_t >::const_iterator i = samplerTextures.begin(); i != samplerTextures.end(); ++i)
	{
		ProgramSampler sampler;
		sampler.stage = i->second;
		sampler.texture = outOffset;

		std::map< std::wstring, uint32_t >::const_iterator j = outTextureParameterMap.find(i->first);
		if (j != outTextureParameterMap.end())
			sampler.texture = j->second;
		else
			outTextureParameterMap[i->first] = outOffset++;

		outSamplers.push_back(sampler);
	}

	return true;
}

bool collectInputSignature(const ShaderGraph* shaderGraph, std::vector< uint8_t >& outSignature)
{
	RefArray< VertexInput > vertexInputNodes;
	shaderGraph->findNodesOf< VertexInput >(vertexInputNodes);

	outSignature.resize(16, 0);

	for (RefArray< VertexInput >::const_iterator i = vertexInputNodes.begin(); i != vertexInputNodes.end(); ++i)
	{
		outSignature[cg_attr_index(
			(*i)->getDataUsage(),
			(*i)->getIndex()
		)] = 1;
	}

	return true;
}

bool compileShader(
	CGCcontext* cgc,
	int32_t optimize,
	const std::wstring& shader,
	bool fragmentProfile,
	uint32_t temporaryRegisterCount,
	CGCbin*& outShaderBin
)
{
	CGCstatus status;

	CGCbin* msg = sceCgcNewBin();
	if (!msg)
		return false;

	outShaderBin = sceCgcNewBin();
	if (!outShaderBin)
		return false;

	char optRegisterCount[32];
	sprintf(optRegisterCount, "%d", temporaryRegisterCount > 0 ? temporaryRegisterCount : 2);

	const char* opt[] = { "-O0", "-O1", "-O2", "-O3", "-O3" };
	const char* argvVertex[] = { opt[optimize], 0 };
	const char* argvFragment[] = { opt[optimize], "--fastmath", "--unroll", "all", "--regcount", optRegisterCount, 0 };

	status = sceCgcCompileString(
		cgc,
		wstombs(shader).c_str(),
		fragmentProfile ? "sce_fp_rsx" : "sce_vp_rsx",
		"main",
		fragmentProfile ? argvFragment : argvVertex,
		outShaderBin,
		msg
	);

	if (status != SCECGC_OK)
	{
		log::error << L"CG shader compile failed" << Endl;
		if (sceCgcGetBinSize(msg) > 1)
			log::error << mbstows((char*)sceCgcGetBinData(msg)) << Endl;
		FormatMultipleLines(log::error, shader);
		sceCgcDeleteBin(msg);
		return false;
	}

	sceCgcDeleteBin(msg);
	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerPs3", 0, ProgramCompilerPs3, IProgramCompiler)

ProgramCompilerPs3::ProgramCompilerPs3()
{
}

ProgramCompilerPs3::~ProgramCompilerPs3()
{
}

const wchar_t* ProgramCompilerPs3::getPlatformSignature() const
{
	return L"GCM";
}

Ref< ProgramResource > ProgramCompilerPs3::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	CGCbin *vertexShaderBin, *pixelShaderBin;

	// Generate CG shaders.
	CgProgram cgProgram;
	if (!Cg().generate(shaderGraph, cgProgram))
		return false;

	// Compile shaders.
	Ref< ProgramResourcePs3 > resource = new ProgramResourcePs3();
	{
		static Semaphore s_globalLock;
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_globalLock);

		CGCcontext* cgc = sceCgcNewContext();
		if (!cgc)
			return 0;

		const std::wstring& vertexShader = cgProgram.getVertexShader();
		const std::wstring& pixelShader = cgProgram.getPixelShader();

		if (!compileShader(cgc, optimize, vertexShader, false, 0, vertexShaderBin))
			return 0;
		if (!compileShader(cgc, optimize, pixelShader, true, cgProgram.getRegisterCount(), pixelShaderBin))
			return 0;

		// Create scalar parameters.
		resource->m_scalarParameterDataSize = 0;

		if (!collectScalarParameters(
			shaderGraph,
			vertexShaderBin,
			false,
			resource->m_vertexScalars,
			resource->m_scalarParameterMap,
			resource->m_scalarParameterDataSize
		))
			return 0;

		if (!collectScalarParameters(
			shaderGraph,
			pixelShaderBin,
			true,
			resource->m_pixelScalars,
			resource->m_scalarParameterMap,
			resource->m_scalarParameterDataSize
		))
			return 0;

		// Create texture parameters.
		resource->m_textureParameterDataSize = 0;

		if (!collectSamplerParameters(
			cgProgram.getVertexTextures(),
			resource->m_vertexSamplers,
			resource->m_textureParameterMap,
			resource->m_textureParameterDataSize
		))
			return 0;

		if (!collectSamplerParameters(
			cgProgram.getPixelTextures(),
			resource->m_pixelSamplers,
			resource->m_textureParameterMap,
			resource->m_textureParameterDataSize
		))
			return 0;

		// Create vertex input signature.
		if (!collectInputSignature(shaderGraph, resource->m_inputSignature))
			return false;

		resource->m_vertexShaderBin = Blob(sceCgcGetBinSize(vertexShaderBin), sceCgcGetBinData(vertexShaderBin));
		resource->m_pixelShaderBin = Blob(sceCgcGetBinSize(pixelShaderBin), sceCgcGetBinData(pixelShaderBin));
		resource->m_renderState = cgProgram.getRenderState();

		// Pretty bad way of measuring cost but we assume size equals cost.
		if (outStats)
		{
			outStats->vertexCost = sceCgcGetBinSize(vertexShaderBin);
			outStats->pixelCost = sceCgcGetBinSize(pixelShaderBin);
		}

		sceCgcDeleteContext(cgc);
	}

	return resource;
}

bool ProgramCompilerPs3::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	std::wstring& outVertexShader,
	std::wstring& outPixelShader
) const
{
	CgProgram cgProgram;
	if (!Cg().generate(shaderGraph, cgProgram))
		return false;

	outVertexShader = cgProgram.getVertexShader();
	outPixelShader = cgProgram.getPixelShader();
	return true;
}

	}
}

#endif
