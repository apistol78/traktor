#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Io/Path.h"

namespace traktor
{

class PropertyGroup;

	namespace render
	{

class IProgramCompiler;
class ShaderGraph;

/*!
 * \ingroup Render
 */
class ProgramCache : public Object
{
	T_RTTI_CLASS;

public:
	ProgramCache(
		const Path& cachePath,
		const IProgramCompiler* compiler
	);

	Ref< ProgramResource > get(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings
	);

private:
	Path m_cachePath;
	Ref< const IProgramCompiler > m_compiler;
};

	}
}