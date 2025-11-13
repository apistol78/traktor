/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderExperiment.h"
#include "Render/Editor/Shader/ShaderExperimentEditorPage.h"
#include "Render/Editor/Shader/ShaderExperimentEditorPageFactory.h"
#include "Ui/Command.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderExperimentEditorPageFactory", 0, ShaderExperimentEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet ShaderExperimentEditorPageFactory::getEditableTypes() const
{
	return makeTypeInfoSet< ShaderExperiment >();
}

bool ShaderExperimentEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IEditorPage > ShaderExperimentEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new ShaderExperimentEditorPage(editor, site, document);
}

void ShaderExperimentEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > ShaderExperimentEditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	return nullptr;
}

}
