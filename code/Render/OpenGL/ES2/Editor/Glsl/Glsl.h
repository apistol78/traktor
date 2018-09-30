/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Glsl_H
#define traktor_render_Glsl_H

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

#endif	// traktor_render_Glsl_H
