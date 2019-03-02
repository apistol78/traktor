#pragma once

#include <vector>
#include "Render/OpenGL/ES2/Editor/Glsl/GlslType.h"

namespace traktor
{

class PropertyGroup;

	namespace render
	{

class ShaderGraph;
class GlslProgram;

/*!
 * \ingroup OGL
 */
class Glsl
{
public:
	bool generate(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		const std::wstring& name,
		GlslProgram& outProgram
	);
};

	}
}

