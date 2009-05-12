#ifndef traktor_render_Glsl_H
#define traktor_render_Glsl_H

#include <vector>

namespace traktor
{
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
		GlslProgram& outProgram
	);
};

	}
}

#endif	// traktor_render_Glsl_H
