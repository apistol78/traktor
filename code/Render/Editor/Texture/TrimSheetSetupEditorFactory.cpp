/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/TrimSheetSetupEditorFactory.h"

#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Texture/TrimSheetSetupAsset.h"
#include "Render/Editor/Texture/TrimSheetSetupEditor.h"
#include "Ui/Command.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TrimSheetSetupEditorFactory", 0, TrimSheetSetupEditorFactory, editor::IEditorPageFactory)

const TypeInfoSet TrimSheetSetupEditorFactory::getEditableTypes() const
{
	return makeTypeInfoSet< TrimSheetSetupAsset >();
}

bool TrimSheetSetupEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IEditorPage > TrimSheetSetupEditorFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new TrimSheetSetupEditor(editor, site, document);
}

void TrimSheetSetupEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Render.TrimSheet.Editor.AddHorizontalSlab"));
	outCommands.push_back(ui::Command(L"Render.TrimSheet.Editor.AddVerticalSlab"));
	outCommands.push_back(ui::Command(L"Render.TrimSheet.Editor.SplitRegion"));
	outCommands.push_back(ui::Command(L"Render.TrimSheet.Editor.Remove"));
	outCommands.push_back(ui::Command(L"Render.TrimSheet.Editor.MoveUp"));
	outCommands.push_back(ui::Command(L"Render.TrimSheet.Editor.MoveDown"));
	outCommands.push_back(ui::Command(L"Render.TrimSheet.Editor.RotateImage"));
	outCommands.push_back(ui::Command(L"Render.TrimSheet.Editor.Reload"));
	outCommands.push_back(ui::Command(L"Render.TrimSheet.Editor.ExportLayer"));
	outCommands.push_back(ui::Command(L"Render.TrimSheet.Editor.ExportLayerGuides"));
	outCommands.push_back(ui::Command(L"Render.TrimSheet.Editor.ExportAllLayers"));
}

Ref< ISerializable > TrimSheetSetupEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

}
