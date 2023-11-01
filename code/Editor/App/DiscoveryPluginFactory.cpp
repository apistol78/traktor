/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/App/DiscoveryPlugin.h"
#include "Editor/App/DiscoveryPluginFactory.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.DiscoveryPluginFactory", 0, DiscoveryPluginFactory, IEditorPluginFactory)

int32_t DiscoveryPluginFactory::getOrdinal() const
{
	return 0;
}

void DiscoveryPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< IEditorPlugin > DiscoveryPluginFactory::createEditorPlugin(IEditor* editor) const
{
	return new DiscoveryPlugin(editor);
}

}
