/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/Editor/MeshAssetRasterizer.h"
#include "Physics/Editor/MeshBrowsePreview.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshBrowsePreview", 0, MeshBrowsePreview, editor::IBrowsePreview)

TypeInfoSet MeshBrowsePreview::getPreviewTypes() const
{
	return makeTypeInfoSet< MeshAsset >();
}

Ref< ui::IBitmap > MeshBrowsePreview::generate(editor::IEditor* editor, db::Instance* instance) const
{
	Ref< const MeshAsset > asset = instance->getObject< MeshAsset >();
	if (!asset)
		return nullptr;

	Ref< drawing::Image > meshThumb = new drawing::Image(
		drawing::PixelFormat::getR8G8B8A8(),
		128,
		128
	);
	meshThumb->clear(Color4f(0.4f, 0.6f, 0.4f, 0.0f));

	MeshAssetRasterizer().generate(editor, asset, meshThumb);

	drawing::ScaleFilter scaleFilter(
		64,
		64,
		drawing::ScaleFilter::MnAverage,
		drawing::ScaleFilter::MgLinear
	);
	meshThumb->apply(&scaleFilter);

	return new ui::Bitmap(meshThumb);
}

}
