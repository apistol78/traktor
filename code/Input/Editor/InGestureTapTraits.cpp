/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/Binding/InGestureTap.h"
#include "Input/Editor/InGestureTapTraits.h"

namespace traktor::input
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
		inGestureTap->m_sourceActive = nullptr;
	else if (inputName == L"X")
		inGestureTap->m_sourceX = nullptr;
	else if (inputName == L"Y")
		inGestureTap->m_sourceY = nullptr;
	else if (inputName == L"Fixed X")
		inGestureTap->m_fixedX = nullptr;
	else if (inputName == L"Fixed Y")
		inGestureTap->m_fixedY = nullptr;
}

}
