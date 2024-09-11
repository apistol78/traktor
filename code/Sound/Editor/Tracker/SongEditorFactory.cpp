/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Sound/Editor/Tracker/SongAsset.h"
#include "Sound/Editor/Tracker/SongEditor.h"
#include "Sound/Editor/Tracker/SongEditorFactory.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SongEditorFactory", 0, SongEditorFactory, editor::IEditorPageFactory)

const TypeInfoSet SongEditorFactory::getEditableTypes() const
{
	return makeTypeInfoSet< SongAsset >();
}

bool SongEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return true;
}

Ref< editor::IEditorPage > SongEditorFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new SongEditor(editor, site, document);
}

void SongEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > SongEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

}
