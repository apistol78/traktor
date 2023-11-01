/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Editor/SoundEditorPlugin.h"
#include "Sound/Editor/SoundEditorPluginFactory.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundEditorPluginFactory", 0, SoundEditorPluginFactory, editor::IEditorPluginFactory)

int32_t SoundEditorPluginFactory::getOrdinal() const
{
	return 0;
}

void SoundEditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< editor::IEditorPlugin > SoundEditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
	return new SoundEditorPlugin(editor);
}

}
