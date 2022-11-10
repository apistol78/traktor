/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/Image2/ImageGraphAsset.h"
#include "Render/Editor/Image2/ImageGraphEditorPage.h"
#include "Render/Editor/Image2/ImageGraphEditorPageFactory.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageGraphEditorPageFactory", 0, ImageGraphEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet ImageGraphEditorPageFactory::getEditableTypes() const
{
	return makeTypeInfoSet< ImageGraphAsset >();
}

bool ImageGraphEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IEditorPage > ImageGraphEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new ImageGraphEditorPage(editor, site, document);
}

void ImageGraphEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"ImageGraph.Editor.AlignBottom"));
	outCommands.push_back(ui::Command(L"ImageGraph.Editor.AlignLeft"));
	outCommands.push_back(ui::Command(L"ImageGraph.Editor.AlignRight"));
	outCommands.push_back(ui::Command(L"ImageGraph.Editor.AlignTop"));
	outCommands.push_back(ui::Command(L"ImageGraph.Editor.Center"));
	outCommands.push_back(ui::Command(L"ImageGraph.Editor.EvenSpaceHorizontally"));
	outCommands.push_back(ui::Command(L"ImageGraph.Editor.EvenSpaceVertically"));
	outCommands.push_back(ui::Command(L"ImageGraph.Editor.AddPass"));
	outCommands.push_back(ui::Command(L"ImageGraph.Editor.AddTarget"));
}

Ref< ISerializable > ImageGraphEditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	Ref< ImageGraphAsset > imageGraph = DeepClone(asset).create< ImageGraphAsset >();
	if (!imageGraph)
		return nullptr;

	for (auto node : imageGraph->getNodes())
		node->setId(Guid::create());

	return imageGraph;
}

	}
}
