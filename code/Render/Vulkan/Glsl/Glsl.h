#ifndef traktor_render_Glsl_H
#define traktor_render_Glsl_H

#include <vector>

namespace traktor
{

class PropertyGroup;

	namespace render
	{

class ShaderGraph;
class GlslProgram;

/*!
 * \ingroup Vulkan
 */
class Glsl
{
public:
	bool generate(
		const ShaderGraph* shaderGraph,
		const PropertyGroup* settings,
		GlslProgram& outProgram
	);
};

	}
}

#endif	// traktor_render_Glsl_H
