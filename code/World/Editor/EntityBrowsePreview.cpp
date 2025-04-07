#pragma optimize("", off)

/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Editor/EntityBrowsePreview.h"

#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IEditor.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/PipelineDependency.h"
#include "Editor/PipelineDependencySet.h"
#include "Ui/Bitmap.h"
#include "World/EntityData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.EntityBrowsePreview", 0, EntityBrowsePreview, editor::IBrowsePreview)

EntityBrowsePreview::EntityBrowsePreview()
{
	for (auto typeInfo : type_of< IBrowsePreview >().findAllOf(false))
	{
		if (!is_type_a< EntityBrowsePreview >(*typeInfo))
		{
			Ref< const IBrowsePreview > browsePreview = mandatory_non_null_type_cast< const IBrowsePreview* >(typeInfo->createInstance());
			m_browsePreview.push_back(browsePreview);
		}
	}
}

TypeInfoSet EntityBrowsePreview::getPreviewTypes() const
{
	return makeTypeInfoSet< EntityData >();
}

Ref< ui::IBitmap > EntityBrowsePreview::generate(editor::IEditor* editor, db::Instance* instance) const
{
	Ref< const EntityData > asset = instance->getObject< EntityData >();
	if (!asset)
		return nullptr;

	// Get all dependencies from entity data.
	editor::PipelineDependencySet dependencySet;
	Ref< editor::IPipelineDepends > depends = editor->createPipelineDepends(&dependencySet, std::numeric_limits< uint32_t >::max());
	if (!depends)
		return nullptr;

	depends->addDependency(asset);
	depends->waitUntilFinished();

	// Try to generate a preview image from any of the dependencies.
	for (uint32_t i = 0; i < dependencySet.size(); ++i)
	{
		const editor::PipelineDependency* dependency = dependencySet.get(i);
		dependency->sourceInstanceGuid;

		Ref< db::Instance > sourceInstance = editor->getSourceDatabase()->getInstance(dependency->sourceInstanceGuid);
		if (!sourceInstance)
			continue;

		const TypeInfo* assetType = sourceInstance->getPrimaryType();
		for (auto browsePreview : m_browsePreview)
		{
			const TypeInfoSet previewTypes = browsePreview->getPreviewTypes();
			if (previewTypes.find(assetType) != previewTypes.end())
				return browsePreview->generate(editor, sourceInstance);
		}
	}

	return nullptr;
}

}
