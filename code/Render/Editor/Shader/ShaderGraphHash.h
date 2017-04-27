/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ShaderGraphHash_H
#define traktor_render_ShaderGraphHash_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Node;
class ShaderGraph;

/*! \brief Shader graph hash.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphHash : public Object
{
	T_RTTI_CLASS;

public:
	static uint32_t calculate(const Node* node);

	static uint32_t calculate(const ShaderGraph* shaderGraph);
};

	}
}

#endif	// traktor_render_ShaderGraphHash_H
