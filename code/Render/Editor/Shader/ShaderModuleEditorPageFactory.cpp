/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Shader/ShaderModule.h"
#include "Render/Editor/Shader/ShaderModuleEditorPage.h"
#include "Render/Editor/Shader/ShaderModuleEditorPageFactory.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderModuleEditorPageFactory", 0, ShaderModuleEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet ShaderModuleEditorPageFactory::getEditableTypes() const
{
	return makeTypeInfoSet< ShaderModule >();
}

bool ShaderModuleEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IEditorPage > ShaderModuleEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new ShaderModuleEditorPage(editor, site, document);
}

void ShaderModuleEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > ShaderModuleEditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

}
