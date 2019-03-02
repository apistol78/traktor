#pragma once

#include <vector>

namespace traktor
{
	namespace render
	{

class PsslProgram;
class ShaderGraph;

/*!
 * \ingroup GNM
 */
class Pssl
{
public:
	bool generate(
		const ShaderGraph* shaderGraph,
		PsslProgram& outProgram
	);
};

	}
}

