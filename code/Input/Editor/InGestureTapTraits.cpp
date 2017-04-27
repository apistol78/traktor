/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Input/Binding/InGestureTap.h"
#include "Input/Editor/InGestureTapTraits.h"

namespace traktor
{
	namespace input
	{

std::wstring InGestureTapTraits::getHeader(const IInputNode* node) const
{
	return L"Gesture Tap";
}

std::wstring InGestureTapTraits::getDescription(const IInputNode* node) const
{
	return L"";
}

Ref< IInputNode > InGestureTapTraits::createNode() const
{
	return new InGestureTap();
}

void InGestureTapTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InGestureTap* inGestureTap = checked_type_cast< const InGestureTap*, false >(node);
	outInputNodes[L"Active"] = inGestureTap->m_sourceActive;
	outInputNodes[L"X"] = inGestureTap->m_sourceX;
	outInputNodes[L"Y"] = inGestureTap->m_sourceY;
	outInputNodes[L"Fixed X"] = inGestureTap->m_fixedX;
	outInputNodes[L"Fixed Y"] = inGestureTap->m_fixedY;
}

void InGestureTapTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InGestureTap* inGestureTap = checked_type_cast< InGestureTap*, false >(node);
	if (inputName == L"Active")
		inGestureTap->m_sourceActive = sourceNode;
	else if (inputName == L"X")
		inGestureTap->m_sourceX = sourceNode;
	else if (inputName == L"Y")
		inGestureTap->m_sourceY = sourceNode;
	else if (inputName == L"Fixed X")
		inGestureTap->m_fixedX = sourceNode;
	else if (inputName == L"Fixed Y")
		inGestureTap->m_fixedY = sourceNode;
}

void InGestureTapTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InGestureTap* inGestureTap = checked_type_cast< InGestureTap*, false >(node);
	if (inputName == L"Active")
		inGestureTap->m_sourceActive = 0;
	else if (inputName == L"X")
		inGestureTap->m_sourceX = 0;
	else if (inputName == L"Y")
		inGestureTap->m_sourceY = 0;
	else if (inputName == L"Fixed X")
		inGestureTap->m_fixedX = 0;
	else if (inputName == L"Fixed Y")
		inGestureTap->m_fixedY = 0;
}

	}
}
