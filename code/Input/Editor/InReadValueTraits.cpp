/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/Binding/InReadValue.h"
#include "Input/Editor/InReadValueTraits.h"

namespace traktor::input
{

std::wstring InReadValueTraits::getHeader(const IInputNode* node) const
{
	return L"Read Value";
}

std::wstring InReadValueTraits::getDescription(const IInputNode* node) const
{
	const InReadValue* inReadValue = checked_type_cast< const InReadValue*, false >(node);
	return inReadValue->getValueId();
}

Ref< IInputNode > InReadValueTraits::createNode() const
{
	return new InReadValue();
}

void InReadValueTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
}

void InReadValueTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
}

void InReadValueTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
}

}
