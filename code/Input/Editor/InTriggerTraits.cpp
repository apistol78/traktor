#include "Input/Binding/InTrigger.h"
#include "Input/Editor/InTriggerTraits.h"

namespace traktor
{
	namespace input
	{

std::wstring InTriggerTraits::getHeader(const IInputNode* node) const
{
	return L"Trigger";
}

std::wstring InTriggerTraits::getDescription(const IInputNode* node) const
{
	return L"";
}

Ref< IInputNode > InTriggerTraits::createNode() const
{
	return new InTrigger();
}

void InTriggerTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InTrigger* inTrigger = checked_type_cast< const InTrigger*, false >(node);
	outInputNodes[L"Input"] = inTrigger->m_source;
}

void InTriggerTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InTrigger* inTrigger = checked_type_cast< InTrigger*, false >(node);
	inTrigger->m_source = sourceNode;
}

void InTriggerTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InTrigger* inTrigger = checked_type_cast< InTrigger*, false >(node);
	inTrigger->m_source = 0;
}

	}
}
