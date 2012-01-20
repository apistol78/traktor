#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
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
	int32_t optimize,
	bool validate,
	IProgramHints* hints,
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

	// Dump generated program.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		if (!m_dump)
			m_dump = FileSystem::getInstance().open(L"Shader.dump", File::FmWrite);

		if (m_dump)
		{
			FileOutputStream fos(m_dump, new Utf8Encoding());

			fos << L"-----------------------------------------------------------------------------------------" << Endl;
			fos << L"Vertex program:" << Endl;
			cx.getVertexProgram().dump(fos, parameters.uniforms);
			fos << Endl;

			fos << L"Pixel program:" << Endl;
			cx.getPixelProgram().dump(fos, parameters.uniforms);
			fos << Endl;

			fos << L"Parameter map:" << Endl;
			for (std::map< std::wstring, std::pair< int32_t, int32_t > >::const_iterator i = parameterMap.begin(); i != parameterMap.end(); ++i)
				fos << L"\"" << i->first << L"\", register " << i->second.first << L", size " << i->second.second << Endl;
			fos << Endl;

			fos << L"Sampler map:" << Endl;
			for (std::map< std::wstring, int32_t >::const_iterator i = samplerMap.begin(); i != samplerMap.end(); ++i)
				fos << L"\"" << i->first << L"\", index " << i->second << Endl;
			fos << Endl;
		}
	}

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

	}
}
