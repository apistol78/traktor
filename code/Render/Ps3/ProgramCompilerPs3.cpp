#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Render/ShaderGraph.h"
#include "Render/Nodes.h"
#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/Cg.h"
#include "Render/Ps3/CgProgram.h"
#include "Render/Ps3/ProgramResourcePs3.h"
#include "Render/Ps3/ProgramCompilerPs3.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct ParameterLess
{
	bool operator () (const ProgramResourcePs3::Parameter& p1, const ProgramResourcePs3::Parameter& p2) const
	{
		return p1.name < p2.name;
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerPs3", 0, ProgramCompilerPs3, IProgramCompiler)

Ref< ProgramResource > ProgramCompilerPs3::compile(const ShaderGraph* shaderGraph, int32_t optimize, bool validate) const
{
	CgProgram cgProgram;
	if (!Cg().generate(shaderGraph, cgProgram))
		return false;

	static Semaphore s_globalLock;

	const char* argv[] = { "-O3", "--fastmath", "--fastprecision", 0 };
	CGCstatus status;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_globalLock);
	
	const std::wstring& vertexShader = cgProgram.getVertexShader();
	const std::wstring& pixelShader = cgProgram.getPixelShader();

	CGCcontext* cgc = sceCgcNewContext();
	if (!cgc)
		return 0;

	CGCbin* msg = sceCgcNewBin();
	if (!msg)
		return 0;

	CGCbin* vertexShaderBin = sceCgcNewBin();
	if (!vertexShaderBin)
		return 0;

	status = sceCgcCompileString(
		cgc,
		wstombs(vertexShader).c_str(),
		"sce_vp_rsx",
		"main",
		argv,
		vertexShaderBin,
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

	CGCbin* pixelShaderBin = sceCgcNewBin();
	if (!pixelShaderBin)
		return 0;

	status = sceCgcCompileString(
		cgc,
		wstombs(pixelShader).c_str(),
		"sce_fp_rsx",
		"main",
		argv,
		pixelShaderBin,
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

	sceCgcDeleteContext(cgc);
	sceCgcDeleteBin(msg);

	// Collect information about parameters which are hard to extract
	// through CG reflection.
	std::set< ProgramResourcePs3::Parameter, ParameterLess > parameters;
	std::vector< ProgramResourcePs3::Sampler > vertexSamplers;
	std::vector< ProgramResourcePs3::Sampler > pixelSamplers;

	RefArray< Uniform > uniformNodes;
	RefArray< IndexedUniform > indexedUniformNodes;

	shaderGraph->findNodesOf< Uniform >(uniformNodes);
	shaderGraph->findNodesOf< IndexedUniform >(indexedUniformNodes);

	for (RefArray< Uniform >::const_iterator i = uniformNodes.begin(); i != uniformNodes.end(); ++i)
	{
		struct ProgramResourcePs3::Parameter param =
		{
			(*i)->getParameterName(),
			0,
			1
		};

		switch ((*i)->getParameterType())
		{
		case PtScalar:
			param.size = 1;
			break;

		case PtVector:
			param.size = 4;
			break;

		case PtMatrix:
			param.size = 16;
			break;

		default:
			T_FATAL_ERROR;
		}

		parameters.insert(param);
	}

	for (RefArray< IndexedUniform >::const_iterator i = indexedUniformNodes.begin(); i != indexedUniformNodes.end(); ++i)
	{
		struct ProgramResourcePs3::Parameter param =
		{
			(*i)->getParameterName(),
			0,
			(*i)->getLength()
		};

		switch ((*i)->getParameterType())
		{
		case PtScalar:
			param.size = 1;
			break;

		case PtVector:
			param.size = 4;
			break;

		case PtMatrix:
			param.size = 16;
			break;

		default:
			T_FATAL_ERROR;
		}

		parameters.insert(param);
	}

	const std::map< std::wstring, uint32_t >& cgVertexSamplers = cgProgram.getVertexSamplers();
	for (std::map< std::wstring, uint32_t >::const_iterator i = cgVertexSamplers.begin(); i != cgVertexSamplers.end(); ++i)
	{
		ProgramResourcePs3::Sampler sampler =
		{
			i->first,
			i->second
		};
		vertexSamplers.push_back(sampler);
	}

	const std::map< std::wstring, uint32_t >& cgPixelSamplers = cgProgram.getPixelSamplers();
	for (std::map< std::wstring, uint32_t >::const_iterator i = cgPixelSamplers.begin(); i != cgPixelSamplers.end(); ++i)
	{
		ProgramResourcePs3::Sampler sampler =
		{
			i->first,
			i->second
		};
		pixelSamplers.push_back(sampler);
	}

	return new ProgramResourcePs3(
		vertexShaderBin,
		pixelShaderBin,
		std::vector< ProgramResourcePs3::Parameter >(parameters.begin(), parameters.end()),
		vertexSamplers,
		pixelSamplers,
		cgProgram.getRenderState()
	);
}

	}
}
