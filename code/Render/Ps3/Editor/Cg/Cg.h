/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Cg_H
#define traktor_render_Cg_H

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

#endif	// traktor_render_Cg_H
