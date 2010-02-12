#include "Render/Ps3/PlatformPs3.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Ps3/Cg.h"
#include "Render/Ps3/CgProgram.h"
#include "Render/Ps3/ProgramResourcePs3.h"
#include "Render/Ps3/ProgramCompilerPs3.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/ShaderGraphOptimizer.h"
#include "Render/Shader/ShaderGraphStatic.h"

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
	std::map< std::wstring, uint32_t >& outScalarParameterMap,
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
		int32_t parameterSize = -1;
		int32_t parameterCount = -1;

		if (const Uniform* uniformNode = dynamic_type_cast< const Uniform* >(*i))
		{
			parameterName = uniformNode->getParameterName();
			switch (uniformNode->getParameterType())
			{
			case PtScalar:
				parameterSize = 1;
				parameterCount = 1;
				break;

			case PtVector:
				parameterSize = 4;
				parameterCount = 1;
				break;

			case PtMatrix:
				parameterSize = 16;
				parameterCount = 1;
				break;
			}
		}
		else if (const IndexedUniform* indexedUniformNode = dynamic_type_cast< const IndexedUniform* >(*i))
		{
			parameterName = indexedUniformNode->getParameterName();
			switch (indexedUniformNode->getParameterType())
			{
			case PtScalar:
				parameterSize = 1;
				parameterCount = indexedUniformNode->getLength();
				break;

			case PtVector:
				parameterSize = 4;
				parameterCount = indexedUniformNode->getLength();
				break;

			case PtMatrix:
				parameterSize = 16;
				parameterCount = indexedUniformNode->getLength();
				break;
			}
		}

		if (parameterSize > 0)
		{
			log::debug << L"Parameter \"" << parameterName << L"\", size = " << parameterSize << L", count = " << parameterCount << Endl;

			uint32_t quadCount = (parameterSize * parameterCount + 3) / 4;

			ProgramScalar scalar;
			scalar.vertexRegisterIndex = 0;
			scalar.vertexRegisterCount = 0;
			scalar.offset = outOffset;
			scalar.length = parameterSize * parameterCount;

			bool scalarUsed = false;

			if (!isFragmentProfile)
			{
				std::string tmp = wstombs(parameterName);
				CGparameter parameter = cellGcmCgGetNamedParameter(program, tmp.c_str());

				if (parameter)
				{
					uint32_t resourceIndex = cellGcmCgGetParameterResourceIndex(program, parameter);

					scalar.vertexRegisterIndex = resourceIndex;
					scalar.vertexRegisterCount = quadCount;

					log::debug << L"\tvertex register index " << scalar.vertexRegisterIndex << Endl;
					log::debug << L"\tvertex register count " << scalar.vertexRegisterCount << Endl;

					scalarUsed = true;
				}
			}
			else
			{
				for (int32_t j = 0; j < quadCount; ++j)
				{
					CGparameter parameter;
					if (quadCount > 1)
						parameter = cellGcmCgGetNamedParameter(program, wstombs(parameterName + L"[" + toString(j) + L"]").c_str());
					else
						parameter = cellGcmCgGetNamedParameter(program, wstombs(parameterName).c_str());
					
					if (parameter)
					{
						uint32_t constantCount = cellGcmCgGetEmbeddedConstantCount(program, parameter);
						for (uint32_t k = 0; k < constantCount; ++k)
						{
							uint32_t constantOffset = cellGcmCgGetEmbeddedConstantOffset(program, parameter, k);

							FragmentOffset fragmentOffset;
							fragmentOffset.ucodeOffset = constantOffset;
							fragmentOffset.parameterOffset = j * 4;

							scalar.fragmentOffsets.push_back(fragmentOffset);

							log::debug << L"\tfragment ucode offset " << fragmentOffset.ucodeOffset << Endl;
							log::debug << L"\tfragment parameter offset " << fragmentOffset.parameterOffset << Endl;
						}

						scalarUsed = true;
					}
				}
			}

			if (scalarUsed)
			{
				std::map< std::wstring, uint32_t >::iterator j = outScalarParameterMap.find(parameterName);
				if (j != outScalarParameterMap.end())
					scalar.offset = j->second;
				else
				{
					outScalarParameterMap[parameterName] = outOffset;
					outOffset += quadCount * 4;
				}

				outScalars.push_back(scalar);
			}
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerPs3", 0, ProgramCompilerPs3, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerPs3::compile(
	const ShaderGraph* shaderGraph,
	int32_t optimize,
	bool validate,
	uint32_t* outCostEstimate
) const
{
	Ref< ShaderGraph > programGraph;

	// Extract platform permutation.
	programGraph = ShaderGraphStatic(shaderGraph).getPlatformPermutation(L"GCM");
	if (!programGraph)
	{
		log::error << L"ProgramCompilerPs3 failed; unable to get platform permutation" << Endl;
		return 0;
	}

	// Freeze type permutation.
	programGraph = ShaderGraphStatic(programGraph).getTypePermutation();
	if (!programGraph)
	{
		log::error << L"ProgramCompilerPs3 failed; unable to get type permutation" << Endl;
		return 0;
	}

	// Merge identical branches.
	programGraph = ShaderGraphOptimizer(programGraph).mergeBranches();
	if (!programGraph)
	{
		log::error << L"ProgramCompilerPs3 failed; unable to merge branches" << Endl;
		return 0;
	}

	// Insert interpolation nodes at optimal locations.
	programGraph = ShaderGraphOptimizer(programGraph).insertInterpolators();
	if (!programGraph)
	{
		log::error << L"ProgramCompilerPs3 failed; unable to optimize shader graph" << Endl;
		return 0;
	}

	// Generate CG shaders.
	CgProgram cgProgram;
	if (!Cg().generate(programGraph, cgProgram))
		return false;

	// Compile shaders.
	Ref< ProgramResourcePs3 > resource = new ProgramResourcePs3();
	{
		static Semaphore s_globalLock;
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_globalLock);

		CGCcontext* cgc = sceCgcNewContext();
		if (!cgc)
			return 0;

		const char* argv[] = { "-O3", "--fastmath", 0 };
		CGCstatus status;

		const std::wstring& vertexShader = cgProgram.getVertexShader();
		const std::wstring& pixelShader = cgProgram.getPixelShader();

		CGCbin* msg = sceCgcNewBin();
		if (!msg)
			return 0;

		resource->m_vertexShaderBin = sceCgcNewBin();
		if (!resource->m_vertexShaderBin)
			return 0;

		status = sceCgcCompileString(
			cgc,
			wstombs(vertexShader).c_str(),
			"sce_vp_rsx",
			"main",
			argv,
			resource->m_vertexShaderBin,
			msg
		);
		if (status != SCECGC_OK)
		{
			log::error << L"Compile CG vertex shader failed" << Endl;
			if (sceCgcGetBinSize(msg) > 1)
				log::error << mbstows((char*)sceCgcGetBinData(msg)) << Endl;
			FormatMultipleLines(log::error, vertexShader);
			return 0;
		}

		resource->m_pixelShaderBin = sceCgcNewBin();
		if (!resource->m_pixelShaderBin)
			return 0;

		status = sceCgcCompileString(
			cgc,
			wstombs(pixelShader).c_str(),
			"sce_fp_rsx",
			"main",
			argv,
			resource->m_pixelShaderBin,
			msg
		);
		if (status != SCECGC_OK)
		{
			log::error << L"Compile CG fragment shader failed" << Endl;
			if (sceCgcGetBinSize(msg) > 1)
				log::error << mbstows((char*)sceCgcGetBinData(msg)) << Endl;
			FormatMultipleLines(log::error, pixelShader);
			return 0;
		}

		sceCgcDeleteBin(msg);

		// Create scalar parameters.
		resource->m_scalarParameterDataSize = 0;

		if (!collectScalarParameters(
			programGraph,
			resource->m_vertexShaderBin,
			false,
			resource->m_vertexScalars,
			resource->m_scalarParameterMap,
			resource->m_scalarParameterDataSize
		))
			return 0;

		if (!collectScalarParameters(
			programGraph,
			resource->m_pixelShaderBin,
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

		resource->m_renderState = cgProgram.getRenderState();

		sceCgcDeleteContext(cgc);
	}

	return resource;
}

	}
}
