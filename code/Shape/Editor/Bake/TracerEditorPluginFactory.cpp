/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Bake/TracerEditorPlugin.h"
#include "Shape/Editor/Bake/TracerEditorPluginFactory.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.TracerEditorPluginFactory", 0, TracerEditorPluginFactory, editor::IEditorPluginFactory)

int32_t TracerEditorPluginFactory::getOrdinal() const
{
	return 100;
}

void TracerEditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< editor::IEditorPlugin > TracerEditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
    return new TracerEditorPlugin(editor);
}

}
