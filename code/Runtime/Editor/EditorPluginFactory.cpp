/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializable.h"
#include "Runtime/Editor/EditorPluginFactory.h"
#include "Runtime/Editor/EditorPlugin.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.EditorPluginFactory", 0, EditorPluginFactory, editor::IEditorPluginFactory)

int32_t EditorPluginFactory::getOrdinal() const
{
	return 100;
}

void EditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Runtime.Editor.LaunchLast"));
}

Ref< editor::IEditorPlugin > EditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
	return new EditorPlugin(editor);
}

}
