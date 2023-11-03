/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/Binding/InThreshold.h"
#include "Input/Editor/InThresholdTraits.h"

namespace traktor::input
{

std::wstring InThresholdTraits::getHeader(const IInputNode* node) const
{
	return L"Threshold";
}

std::wstring InThresholdTraits::getDescription(const IInputNode* node) const
{
	return L"";
}

Ref< IInputNode > InThresholdTraits::createNode() const
{
	return new InThreshold();
}

void InThresholdTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InThreshold* inThreshold = checked_type_cast< const InThreshold*, false >(node);
	outInputNodes[L"Input"] = inThreshold->m_source;
}

void InThresholdTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InThreshold* inThreshold = checked_type_cast< InThreshold*, false >(node);
	inThreshold->m_source = sourceNode;
}

void InThresholdTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InThreshold* inThreshold = checked_type_cast< InThreshold*, false >(node);
	inThreshold->m_source = nullptr;
}

}
