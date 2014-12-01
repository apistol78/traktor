#include "Input/Binding/InGestureSwipe.h"
#include "Input/Editor/InGestureSwipeTraits.h"

namespace traktor
{
	namespace input
	{

std::wstring InGestureSwipeTraits::getHeader(const IInputNode* node) const
{
	return L"Gesture Swipe";
}

std::wstring InGestureSwipeTraits::getDescription(const IInputNode* node) const
{
	const InGestureSwipe* inGestureSwipe = checked_type_cast< const InGestureSwipe*, false >(node);
	switch (inGestureSwipe->m_direction)
	{
	case InGestureSwipe::SdUp:
		return L"Up";
	case InGestureSwipe::SdDown:
		return L"Down";
	case InGestureSwipe::SdLeft:
		return L"Left";
	case InGestureSwipe::SdRight:
		return L"Right";
	default:
		return L"";
	}
}

void InGestureSwipeTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InGestureSwipe* inGestureSwipe = checked_type_cast< const InGestureSwipe*, false >(node);
	outInputNodes[L"Active"] = inGestureSwipe->m_sourceActive;
	outInputNodes[L"X"] = inGestureSwipe->m_sourceX;
	outInputNodes[L"Y"] = inGestureSwipe->m_sourceY;
}

void InGestureSwipeTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InGestureSwipe* inGestureSwipe = checked_type_cast< InGestureSwipe*, false >(node);
	if (inputName == L"Active")
		inGestureSwipe->m_sourceActive = sourceNode;
	else if (inputName == L"X1")
		inGestureSwipe->m_sourceX = sourceNode;
	else if (inputName == L"Y1")
		inGestureSwipe->m_sourceY = sourceNode;
}

void InGestureSwipeTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InGestureSwipe* inGestureSwipe = checked_type_cast< InGestureSwipe*, false >(node);
	if (inputName == L"Active")
		inGestureSwipe->m_sourceActive = 0;
	else if (inputName == L"X1")
		inGestureSwipe->m_sourceX = 0;
	else if (inputName == L"Y1")
		inGestureSwipe->m_sourceY = 0;
}

	}
}
