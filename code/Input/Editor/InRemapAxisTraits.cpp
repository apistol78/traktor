#include "Input/Binding/InRemapAxis.h"
#include "Input/Editor/InRemapAxisTraits.h"

namespace traktor
{
	namespace input
	{

std::wstring InRemapAxisTraits::getHeader(const IInputNode* node) const
{
	return L"Remap Axis";
}

std::wstring InRemapAxisTraits::getDescription(const IInputNode* node) const
{
	return L"";
}

Ref< IInputNode > InRemapAxisTraits::createNode() const
{
	return new InRemapAxis();
}

void InRemapAxisTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InRemapAxis* inRemapAxis = checked_type_cast< const InRemapAxis*, false >(node);
	outInputNodes[L"Input"] = inRemapAxis->m_source;
}

void InRemapAxisTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InRemapAxis* inRemapAxis = checked_type_cast< InRemapAxis*, false >(node);
	inRemapAxis->m_source = sourceNode;
}

void InRemapAxisTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InRemapAxis* inRemapAxis = checked_type_cast< InRemapAxis*, false >(node);
	inRemapAxis->m_source = 0;
}

	}
}
