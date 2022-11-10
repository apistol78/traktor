/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <string>
#include "Core/Object.h"

namespace traktor
{
	namespace input
	{

class IInputNode;

class InputNodeTraits : public Object
{
public:
	virtual std::wstring getHeader(const IInputNode* node) const = 0;

	virtual std::wstring getDescription(const IInputNode* node) const = 0;

	virtual Ref< IInputNode > createNode() const = 0;

	virtual void getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const = 0;

	virtual void connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const = 0;

	virtual void disconnectInputNode(IInputNode* node, const std::wstring& inputName) const = 0;
};

	}
}

