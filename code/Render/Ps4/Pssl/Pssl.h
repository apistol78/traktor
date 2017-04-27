/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Pssl_H
#define traktor_render_Pssl_H

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

#endif	// traktor_render_Pssl_H
