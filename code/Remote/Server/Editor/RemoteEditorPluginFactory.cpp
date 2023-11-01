/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Remote/Server/Editor/RemoteEditorPlugin.h"
#include "Remote/Server/Editor/RemoteEditorPluginFactory.h"

namespace traktor::remote
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.remote.RemoteEditorPluginFactory", 0, RemoteEditorPluginFactory, editor::IEditorPluginFactory)

int32_t RemoteEditorPluginFactory::getOrdinal() const
{
	return 100;
}

void RemoteEditorPluginFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< editor::IEditorPlugin > RemoteEditorPluginFactory::createEditorPlugin(editor::IEditor* editor) const
{
	return new RemoteEditorPlugin(editor);
}

}
