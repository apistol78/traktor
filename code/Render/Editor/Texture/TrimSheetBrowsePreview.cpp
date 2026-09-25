/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/TrimSheetBrowsePreview.h"

#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IEditor.h"
#include "Render/Editor/Texture/TrimSheetComposer.h"
#include "Render/Editor/Texture/TrimSheetSetupAsset.h"
#include "Render/Editor/Texture/TrimSheetTextureAsset.h"
#include "Ui/Bitmap.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TrimSheetBrowsePreview", 0, TrimSheetBrowsePreview, editor::IBrowsePreview)

TypeInfoSet TrimSheetBrowsePreview::getPreviewTypes() const
{
	return makeTypeInfoSet< TrimSheetSetupAsset, TrimSheetTextureAsset >();
}

Ref< ui::IBitmap > TrimSheetBrowsePreview::generate(editor::IEditor* editor, db::Instance* instance, int32_t size) const
{
	const std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	Ref< drawing::Image > image = generateImage(assetPath, editor->getSourceDatabase(), instance->getGuid(), size, size);
	return image ? new ui::Bitmap(image) : nullptr;
}

Ref< drawing::Image > TrimSheetBrowsePreview::generateImage(const std::wstring& assetPath, db::Database* database, const Guid& instanceGuid, int32_t width, int32_t height)
{
	Ref< const ISerializable > object = database->getObjectReadOnly(instanceGuid);
	if (!object)
		return nullptr;

	Ref< const TrimSheetSetupAsset > setup;
	TrimSheetLayer layer = TrimSheetLayer::Albedo;

	if (auto textureAsset = dynamic_type_cast< const TrimSheetTextureAsset* >(object))
	{
		setup = database->getObjectReadOnly< TrimSheetSetupAsset >(textureAsset->getSetup());
		layer = textureAsset->getLayer();
	}
	else
		setup = dynamic_type_cast< const TrimSheetSetupAsset* >(object);

	if (!setup)
		return nullptr;

	Ref< drawing::Image > image = TrimSheetComposer(Path(assetPath)).composeThumbnail(setup, layer, width, height);
	if (!image)
		return nullptr;

	image->clearAlpha(1.0f);
	return image;
}

}
