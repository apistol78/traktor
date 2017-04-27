/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ShaderGraphOrderEvaluator_H
#define traktor_render_ShaderGraphOrderEvaluator_H

#include <map>
#include "Core/Object.h"
#include "Render/Editor/Shader/PinType.h"

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

class ShaderGraph;
class Node;
class OutputPin;

/*! \brief Evaluate algorithmic order of a shader graph node.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphOrderEvaluator : public Object
{
	T_RTTI_CLASS;

public:
	ShaderGraphOrderEvaluator(
		const ShaderGraph* shaderGraph,
		bool frequentUniformsAsLinear
	);

	PinOrderType evaluate(const Node* node, const std::wstring& inputPinName) const;

	PinOrderType evaluate(const OutputPin* outputPin) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	bool m_frequentUniformsAsLinear;
	mutable std::map< const OutputPin*, PinOrderType > m_evaluated;
};

	}
}

#endif	// traktor_render_ShaderGraphOrderEvaluator_H
