#include "Core/Misc/String.h"
#include "Input/Binding/InLowPass.h"
#include "Input/Editor/InLowPassTraits.h"

namespace traktor
{
	namespace input
	{

std::wstring InLowPassTraits::getHeader(const IInputNode* node) const
{
	return L"Low pass";
}

std::wstring InLowPassTraits::getDescription(const IInputNode* node) const
{
	const InLowPass* ilp = checked_type_cast< const InLowPass*, false >(node);
	return L"Low pass " + toString(ilp->m_coeff);
}

Ref< IInputNode > InLowPassTraits::createNode() const
{
	return new InLowPass();
}

void InLowPassTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InLowPass* ilp = checked_type_cast< const InLowPass*, false >(node);
	outInputNodes[L"Input"] = ilp->m_source;
}

void InLowPassTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InLowPass* ilp = checked_type_cast< InLowPass*, false >(node);
	ilp->m_source = sourceNode;
}

void InLowPassTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InLowPass* ilp = checked_type_cast< InLowPass*, false >(node);
	ilp->m_source = 0;
}

	}
}
