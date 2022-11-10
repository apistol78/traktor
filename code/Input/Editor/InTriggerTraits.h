/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Input/Editor/InputNodeTraits.h"

namespace traktor
{
	namespace input
	{

class InTriggerTraits : public InputNodeTraits
{
public:
	virtual std::wstring getHeader(const IInputNode* node) const override final;

	virtual std::wstring getDescription(const IInputNode* node) const override final;

	virtual Ref< IInputNode > createNode() const override final;

	virtual void getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const override final;

	virtual void connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const override final;

	virtual void disconnectInputNode(IInputNode* node, const std::wstring& inputName) const override final;
};

	}
}

