/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Script/Editor/ScriptEditorPlugin.h"
#include "Script/Editor/ScriptEditorPluginFactory.h"

namespace traktor::script
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptEditorPluginFactory", 0, ScriptEditorPluginFactory, editor::IEditorPluginFactory)

int32_t ScriptEditorPluginFactory::getOrdinal() const
{
	return 0;
}

void ScriptEditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< editor::IEditorPlugin > ScriptEditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
	return new ScriptEditorPlugin(editor);
}

}
