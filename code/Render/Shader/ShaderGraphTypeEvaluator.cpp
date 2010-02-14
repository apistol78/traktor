#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Shader/ShaderGraphTypeEvaluator.h"

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
		return PinType();
}

PinType ShaderGraphTypeEvaluator::evaluate(const Node* node, const std::wstring& inputPinName) const
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	if (inputPin)
		return evaluate(inputPin);
	else
		return PinType();
}

PinType ShaderGraphTypeEvaluator::evaluate(const OutputPin* outputPin) const
{
	std::map< const OutputPin*, PinType >::const_iterator i = m_cache.find(outputPin);
	if (i != m_cache.end())
		return i->second;

	m_cache[outputPin] = PinType();

	const Node* node = outputPin->getNode();
	T_ASSERT (node);

	PinType outputPinType;

	if (const IndexedUniform* indexedUniformNode = dynamic_type_cast< const IndexedUniform* >(node))
	{
		switch (indexedUniformNode->getParameterType())
		{
		case PtScalar:
			outputPinType = PinType(PtcScalar);
			break;

		case PtVector:
			outputPinType = PinType(PtcScalar, 4);
			break;

		case PtMatrix:
			outputPinType = PinType(PtcMatrix);
			break;

		case PtTexture:
			outputPinType = PinType(PtcTexture);
			break;
		}
	}
	else if (is_a< Scalar >(node))
	{
		outputPinType = PinType(PtcScalar);
	}
	else if (is_a< Vector >(node) || is_a< Color >(node) || is_a< FragmentPosition >(node))
	{
		outputPinType = PinType(PtcScalar, 4);
	}
	else if (const Swizzle* swizzleNode = dynamic_type_cast< const Swizzle* >(node))
	{
		const std::wstring& pattern = swizzleNode->get();
		if (pattern.length() >= 1 && pattern.length() <= 4)
			outputPinType = PinType(PtcScalar, pattern.length());
		else
			outputPinType = PinType();
	}
	else if (is_a< Conditional >(node))
	{
		outputPinType = PinType::maxPrecedence(
			evaluate(node, L"CaseTrue"),
			evaluate(node, L"CaseFalse")
		);
	}
	else if (is_a< Lerp >(node))
	{
		outputPinType = PinType::maxPrecedence(
			evaluate(node, L"Input1"),
			evaluate(node, L"Input2")
		);
	}
	else if (is_a< Matrix >(node))
	{
		outputPinType = PinType(PtcMatrix);
	}
	else if (is_a< MixIn >(node))
	{
		outputPinType = PinType(PtcScalar, 4);
	}
	else if (is_a< MixOut >(node))
	{
		outputPinType = PinType(PtcScalar);
	}
	else if (is_a< Polynomial >(node))
	{
		outputPinType = PinType(PtcScalar);
	}
	else if (is_a< Sampler >(node))
	{
		outputPinType = PinType(PtcScalar, 4);
	}
	else if (is_a< Iterate >(node) || is_a< Sum >(node))
	{
		if (outputPin == node->findOutputPin(L"N"))
			outputPinType = PinType(PtcScalar, 1);
		else
		{
			const InputPin* inputPin = node->findInputPin(L"Input");
			T_ASSERT (inputPin);

			outputPinType = evaluate(inputPin);
		}
	}
	else if (is_a< Texture >(node))
	{
		outputPinType = PinType(PtcTexture);
	}
	else if (is_a< Transform >(node))
	{
		outputPinType = evaluate(node, L"Input");
	}
	else if (const Type* typeNode = dynamic_type_cast< const Type* >(node))
	{
		PinType typeInputType = evaluate(node, L"Type");

		if (typeInputType.getClass() == PtcScalar)
		{
			if (typeInputType.getWidth() <= 1)
				outputPinType = evaluate(node, L"Scalar");
			else
				outputPinType = evaluate(node, L"Vector");
		}
		else if (typeInputType.getClass() == PtcMatrix)
		{
			outputPinType = evaluate(node, L"Matrix");
		}
		else if (typeInputType.getClass() == PtcTexture)
		{
			outputPinType = evaluate(node, L"Texture");
		}

		if (outputPinType.getClass() == PtcVoid)
			outputPinType = evaluate(node, L"Default");
	}
	else if (const Uniform* uniformNode = dynamic_type_cast< const Uniform* >(node))
	{
		switch (uniformNode->getParameterType())
		{
		case PtScalar:
			outputPinType = PinType(PtcScalar);
			break;

		case PtVector:
			outputPinType = PinType(PtcScalar, 4);
			break;

		case PtMatrix:
			outputPinType = PinType(PtcMatrix);
			break;

		case PtTexture:
			outputPinType = PinType(PtcTexture);
			break;
		}
	}
	else if (const VertexInput* vertexInputNode = dynamic_type_cast< const VertexInput* >(node))
	{
		switch (vertexInputNode->getDataType())
		{
		case DtFloat1:
			outputPinType = PinType(PtcScalar, 1);
			break;

		case DtFloat2:
		case DtShort2:
		case DtShort2N:
		case DtHalf2:
			outputPinType = PinType(PtcScalar, 2);
			break;

		case DtFloat3:
			outputPinType = PinType(PtcScalar, 3);
			break;

		case DtFloat4:
		case DtByte4:
		case DtByte4N:
		case DtShort4:
		case DtShort4N:
		case DtHalf4:
			outputPinType = PinType(PtcScalar, 4);
			break;
		}
	}
	else
	{
		int32_t inputPinCount = node->getInputPinCount();
		for (int32_t i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			T_ASSERT (inputPin);

			PinType inputPinType = evaluate(inputPin);

			outputPinType = PinType::maxPrecedence(
				inputPinType,
				outputPinType
			);
		}
	}

	m_cache[outputPin] = outputPinType;

	return outputPinType;
}

	}
}
