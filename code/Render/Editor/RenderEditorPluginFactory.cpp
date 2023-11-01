/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/RenderEditorPlugin.h"
#include "Render/Editor/RenderEditorPluginFactory.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderEditorPluginFactory", 0, RenderEditorPluginFactory, editor::IEditorPluginFactory)

int32_t RenderEditorPluginFactory::getOrdinal() const
{
	return 0;
}

void RenderEditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Render.PrintMemoryUsage"));
}

Ref< editor::IEditorPlugin > RenderEditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
	return new RenderEditorPlugin(editor);
}

}
