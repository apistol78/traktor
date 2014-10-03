#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Sw/ProgramCompilerSw.h"
#include "Render/Sw/ProgramResourceSw.h"
#include "Render/Sw/Emitter/Emitter.h"
#include "Render/Sw/Emitter/EmitterContext.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramCompilerSw", 0, ProgramCompilerSw, IProgramCompiler)

const wchar_t* ProgramCompilerSw::getPlatformSignature() const
{
	return L"Software";
}

Ref< ProgramResource > ProgramCompilerSw::compile(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	bool validate,
	Stats* outStats
) const
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate program; incorrect number of outputs" << Endl;
		return 0;
	}

	EmitterContext::Parameters parameters;
	EmitterContext cx(shaderGraph, parameters);

	// Emit outputs.
	cx.emit(pixelOutputs[0]);
	cx.emit(vertexOutputs[0]);

	// Get emitter parameters.
	std::map< std::wstring, std::pair< int32_t, int32_t > > parameterMap;
	for (std::map< std::wstring, Variable* >::const_iterator i = parameters.uniforms.begin(); i != parameters.uniforms.end(); ++i)
		parameterMap[i->first] = std::make_pair(i->second->reg, i->second->size);

	std::map< std::wstring, int32_t > samplerMap;
	for (std::map< std::wstring, int32_t >::const_iterator i = parameters.samplers.begin(); i != parameters.samplers.end(); ++i)
		samplerMap[i->first] = i->second;

	Ref< ProgramResourceSw > resource = new ProgramResourceSw(
		cx.getVertexProgram(),
		cx.getPixelProgram(),
		parameterMap,
		samplerMap,
		cx.getRenderState(),
		cx.getInterpolatorCount()
	);

	return resource;
}

bool ProgramCompilerSw::generate(
	const ShaderGraph* shaderGraph,
	const PropertyGroup* settings,
	int32_t optimize,
	std::wstring& outShader
) const
{
	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
		return false;

	EmitterContext::Parameters parameters;
	EmitterContext cx(shaderGraph, parameters);

	// Emit outputs.
	cx.emit(pixelOutputs[0]);
	cx.emit(vertexOutputs[0]);

	// Get emitter parameters.
	std::map< std::wstring, std::pair< int32_t, int32_t > > parameterMap;
	for (std::map< std::wstring, Variable* >::const_iterator i = parameters.uniforms.begin(); i != parameters.uniforms.end(); ++i)
		parameterMap[i->first] = std::make_pair(i->second->reg, i->second->size);

	std::map< std::wstring, int32_t > samplerMap;
	for (std::map< std::wstring, int32_t >::const_iterator i = parameters.samplers.begin(); i != parameters.samplers.end(); ++i)
		samplerMap[i->first] = i->second;

	StringOutputStream ss;

	ss << L"Vertex program:" << Endl;
	cx.getVertexProgram().dump(ss, parameters.uniforms);
	ss << Endl;

	ss << L"Pixel program:" << Endl;
	cx.getPixelProgram().dump(ss, parameters.uniforms);
	ss << Endl;

	ss << L"Parameter map:" << Endl;
	for (std::map< std::wstring, std::pair< int32_t, int32_t > >::const_iterator i = parameterMap.begin(); i != parameterMap.end(); ++i)
		ss << L"\"" << i->first << L"\", register " << i->second.first << L", size " << i->second.second << Endl;
	ss << Endl;

	ss << L"Sampler map:" << Endl;
	for (std::map< std::wstring, int32_t >::const_iterator i = samplerMap.begin(); i != samplerMap.end(); ++i)
		ss << L"\"" << i->first << L"\", index " << i->second << Endl;
	ss << Endl;

	outShader = ss.str();
	return true;
}

	}
}
