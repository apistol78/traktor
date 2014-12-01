#include "Core/Misc/String.h"
#include "Input/Binding/InPulse.h"
#include "Input/Editor/InPulseTraits.h"

namespace traktor
{
	namespace input
	{

std::wstring InPulseTraits::getHeader(const IInputNode* node) const
{
	return L"Pulse";
}

std::wstring InPulseTraits::getDescription(const IInputNode* node) const
{
	const InPulse* inPulse = checked_type_cast< const InPulse*, false >(node);
	return 
		L"Delay " + toString(inPulse->m_delay) + L"\n" +
		L"Interval " + toString(inPulse->m_interval);
}

void InPulseTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InPulse* inPulse = checked_type_cast< const InPulse*, false >(node);
	outInputNodes[L"Input"] = inPulse->m_source;
}

void InPulseTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InPulse* inPulse = checked_type_cast< InPulse*, false >(node);
	inPulse->m_source = sourceNode;
}

void InPulseTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InPulse* inPulse = checked_type_cast< InPulse*, false >(node);
	inPulse->m_source = 0;
}

	}
}
