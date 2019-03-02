#include "Core/Misc/String.h"
#include "Input/Binding/InHysteresis.h"
#include "Input/Editor/InHysteresisTraits.h"

namespace traktor
{
	namespace input
	{

std::wstring InHysteresisTraits::getHeader(const IInputNode* node) const
{
	return L"Hysteresis";
}

std::wstring InHysteresisTraits::getDescription(const IInputNode* node) const
{
	const InHysteresis* inHysteresis = checked_type_cast< const InHysteresis*, false >(node);
	return
		L"Limit " + toString(inHysteresis->m_limit[0]) + L" -> " + toString(inHysteresis->m_limit[1]) + L"\n" +
		L"Output " + toString(inHysteresis->m_output[0]) + L" -> " + toString(inHysteresis->m_output[1]);
}

Ref< IInputNode > InHysteresisTraits::createNode() const
{
	return new InHysteresis();
}

void InHysteresisTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InHysteresis* inHysteresis = checked_type_cast< const InHysteresis*, false >(node);
	outInputNodes[L"Input"] = inHysteresis->m_source;
}

void InHysteresisTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InHysteresis* inHysteresis = checked_type_cast< InHysteresis*, false >(node);
	inHysteresis->m_source = sourceNode;
}

void InHysteresisTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InHysteresis* inHysteresis = checked_type_cast< InHysteresis*, false >(node);
	inHysteresis->m_source = 0;
}

	}
}
