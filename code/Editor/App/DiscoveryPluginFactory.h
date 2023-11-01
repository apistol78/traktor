/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/IEditorPluginFactory.h"

namespace traktor::editor
{

class DiscoveryPluginFactory : public IEditorPluginFactory
{
	T_RTTI_CLASS;

public:
	virtual int32_t getOrdinal() const override final;

	virtual void getCommands(std::list< ui::Command >& outCommands) const override final;

	virtual Ref< IEditorPlugin > createEditorPlugin(IEditor* editor) const override final;
};

}
