/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ShaderGraphTypeEvaluator_H
#define traktor_render_ShaderGraphTypeEvaluator_H

#include <map>
#include "Core/Object.h"
#include "Core/Ref.h"
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

class InputPin;
class Node;
class OutputPin;
class ShaderGraph;

/*! \brief Shader graph output pin type evaluator.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphTypeEvaluator : public Object
{
	T_RTTI_CLASS;

public:
	ShaderGraphTypeEvaluator(const ShaderGraph* shaderGraph);

	PinType evaluate(const InputPin* inputPin) const;

	PinType evaluate(const Node* node, const std::wstring& inputPinName) const;

	PinType evaluate(const OutputPin* outputPin) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	mutable std::map< const OutputPin*, PinType > m_cache;
};

	}
}

#endif	// traktor_render_ShaderGraphTypeEvaluator_H
