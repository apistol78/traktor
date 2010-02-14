#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphOrderEvaluator.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphOrderEvaluator", ShaderGraphOrderEvaluator, Object)

ShaderGraphOrderEvaluator::ShaderGraphOrderEvaluator(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
}

int ShaderGraphOrderEvaluator::evaluate(const Node* node, const std::wstring& inputPinName) const
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT (inputPin);

	const OutputPin* sourceOutputPin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourceOutputPin)
		return OrConstant;

	return evaluate(sourceOutputPin->getNode());
}

int ShaderGraphOrderEvaluator::evaluate(const Node* node) const
{
	std::map< const Node*, int >::iterator i = m_evaluated.find(node);
	if (i != m_evaluated.end())
		return i->second;

	// Initialize entry to prevent infinite recursion; if we
	// detect recursion we assume high order.
	m_evaluated[node] = OrNonLinear;

	int order = OrConstant;

	if (
		is_a< ArcusCos >(node) ||
		is_a< Cos >(node) ||
		is_a< Sin >(node) ||
		is_a< Tan >(node)
	)
		order = nodeTrig(node);
	else if (is_a< ArcusTan >(node))
		order = nodeArcusTan(node);
	else if (
		is_a< Color >(node) ||
		is_a< Scalar >(node) ||
		is_a< Uniform >(node) ||
		is_a< Vector >(node)
	)
		order = OrConstant;
	else if (is_a< IndexedUniform >(node))
		order = nodeIndexedUniform(node);
	else if (
		is_a< Abs >(node) ||
		is_a< Clamp >(node) ||
		is_a< Conditional >(node) ||
		is_a< Cross >(node) ||
		is_a< Dot >(node) ||
		is_a< Derivative >(node) ||
		is_a< Exp >(node) ||
		is_a< Fraction >(node) ||
		is_a< Length >(node) ||
		is_a< Log >(node) ||
		is_a< Max >(node) ||
		is_a< Min >(node) ||
		is_a< Normalize >(node) ||
		is_a< Polynomial >(node) ||
		is_a< Pow >(node) ||
		is_a< Reflect >(node) ||
		is_a< Sqrt >(node)
	)
		order = nodeConstantOrNonLinear(node);
	else if(
		is_a< FragmentPosition >(node) ||
		is_a< Sampler >(node)
	)
		order = OrNonLinear;
	else if (is_a< Div >(node))
		order = nodeMulOrDiv(node);
	else if (is_a< Mul >(node))
		order = nodeMulOrDiv(node);
	else if (is_a< MulAdd >(node))
		order = nodeMulAdd(node);
	else if (is_a< Matrix >(node))
		order = nodeMatrix(node);
	else if (is_a< Lerp >(node))
		order = nodeLerp(node);
	else if (is_a< VertexInput >(node))
		order = nodeDefault(node, OrLinear);
	else
		order = nodeDefault(node, OrConstant);

	m_evaluated[node] = order;
	return order;
}

int ShaderGraphOrderEvaluator::nodeDefault(const Node* node, int initialOrder) const
{
	int order = initialOrder;
	for (int i = 0; i < node->getInputPinCount(); ++i)
	{
		const InputPin* inputPin = node->getInputPin(i);
		int pinOrder = evaluate(node, inputPin->getName());
		order = std::max(order, pinOrder);
	}
	return order;
}

int ShaderGraphOrderEvaluator::nodeConstantOrNonLinear(const Node* node) const
{
	for (int i = 0; i < node->getInputPinCount(); ++i)
	{
		const InputPin* inputPin = node->getInputPin(i);
		int pinOrder = evaluate(node, inputPin->getName());
		if (pinOrder != OrConstant)
			return OrNonLinear;
	}
	return OrConstant;
}

int ShaderGraphOrderEvaluator::nodeMulOrDiv(const Node* node) const
{
	int order1 = evaluate(node, L"Input1");
	int order2 = evaluate(node, L"Input2");
	return order1 + order2;
}

int ShaderGraphOrderEvaluator::nodeMulAdd(const Node* node) const
{
	int order1 = evaluate(node, L"Input1");
	int order2 = evaluate(node, L"Input2");
	int order3 = evaluate(node, L"Input3");
	return std::max(order1 + order2, order3);
}

int ShaderGraphOrderEvaluator::nodeTrig(const Node* node) const
{
	int order = evaluate(node, L"Theta");
	return order <= 0 ? OrConstant : OrNonLinear;
}

int ShaderGraphOrderEvaluator::nodeArcusTan(const Node* node) const
{
	int order = evaluate(node, L"XY");
	return order <= 0 ? OrConstant : OrNonLinear;
}

int ShaderGraphOrderEvaluator::nodeIndexedUniform(const Node* node) const
{
	int order = evaluate(node, L"Index");
	return order <= 0 ? OrConstant : OrNonLinear;
}

int ShaderGraphOrderEvaluator::nodeMatrix(const Node* node) const
{
	int order1 = evaluate(node, L"XAxis");
	int order2 = evaluate(node, L"YAxis");
	int order3 = evaluate(node, L"ZAxis");
	int order4 = evaluate(node, L"Translate");

	int order = std::max(order1, order2);
	order = std::max(order, order3);
	order = std::max(order, order4);

	return order <= 0 ? OrConstant : OrNonLinear;
}

int ShaderGraphOrderEvaluator::nodeLerp(const Node* node) const
{
	int order1 = evaluate(node, L"Input1");
	int order2 = evaluate(node, L"Input2");
	int order3 = evaluate(node, L"Blend");
	return std::max(order1 + order3, order2 + order3);
}

	}
}
