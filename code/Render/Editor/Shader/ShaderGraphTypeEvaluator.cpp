/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Shader/Nodes.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/ShaderGraphTypeEvaluator.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphTypeEvaluator", ShaderGraphTypeEvaluator, Object)

ShaderGraphTypeEvaluator::ShaderGraphTypeEvaluator(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
}

PinType ShaderGraphTypeEvaluator::evaluate(const InputPin* inputPin) const
{
	const OutputPin* outputPin = m_shaderGraph->findSourcePin(inputPin);
	if (outputPin)
		return evaluate(outputPin);
	else
		return PntVoid;
}

PinType ShaderGraphTypeEvaluator::evaluate(const Node* node, const std::wstring& inputPinName) const
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	if (inputPin)
		return evaluate(inputPin);
	else
		return PntVoid;
}

PinType ShaderGraphTypeEvaluator::evaluate(const OutputPin* outputPin) const
{
	std::map< const OutputPin*, PinType >::const_iterator i = m_cache.find(outputPin);
	if (i != m_cache.end())
		return i->second;

	PinType outputPinType = PntVoid;
	m_cache[outputPin] = PntVoid;

	const Node* node = outputPin->getNode();
	T_ASSERT (node);

	if (is_a< Type >(node))
	{
		PinType inputType = evaluate(node, L"Type");

		const InputPin* inputPin = 0;
		if (isPinTypeScalar(inputType))
		{
			if (getPinTypeWidth(inputType) <= 1)
				inputPin = node->findInputPin(L"Scalar");
			else
				inputPin = node->findInputPin(L"Vector");
		}
		else if (inputType == PntMatrix)
		{
			inputPin = node->findInputPin(L"Matrix");
		}
		else if (isPinTypeTexture(inputType))
		{
			inputPin = node->findInputPin(L"Texture");
		}
		if (!inputPin)
			return PntVoid;

		outputPinType = evaluate(inputPin);
	}
	else
	{
		const INodeTraits* nodeTraits = INodeTraits::find(node);
		T_ASSERT (nodeTraits);

		uint32_t inputPinCount = node->getInputPinCount();

		// Evaluate input pin types.
		std::vector< PinType > inputPinTypes(inputPinCount);
		for (uint32_t i = 0; i < inputPinCount; ++i)
			inputPinTypes[i] = evaluate(node->getInputPin(i));

		// Determine output pin type from trait.
		outputPinType = nodeTraits->getOutputPinType(
			node,
			outputPin,
			inputPinCount > 0 ? &inputPinTypes[0] : 0
		);
	}

	m_cache[outputPin] = outputPinType;
	return outputPinType;
}

	}
}
