#pragma once

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class CgProgram;

/*!
 * \ingroup PS3
 */
class Cg
{
public:
	bool generate(
		const ShaderGraph* shaderGraph,
		CgProgram& outProgram
	);
};

	}
}

