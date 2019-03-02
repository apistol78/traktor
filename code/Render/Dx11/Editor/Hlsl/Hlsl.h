#pragma once

#include <vector>

namespace traktor
{
	namespace render
	{

class HlslProgram;
class ShaderGraph;

/*!
 * \ingroup DX11
 */
class Hlsl
{
public:
	bool generate(
		const ShaderGraph* shaderGraph,
		HlslProgram& outProgram
	);
};

	}
}

