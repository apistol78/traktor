/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Hlsl_H
#define traktor_render_Hlsl_H

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

#endif	// traktor_render_Hlsl_H
