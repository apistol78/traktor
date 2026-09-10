/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/GaussianBlurFilter.h"
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

Ref< ui::IBitmap > MeshBrowsePreview::generate(editor::IEditor* editor, db::Instance* instance, int32_t size) const
{
	Ref< const MeshAsset > asset = instance->getObject< MeshAsset >();
	if (!asset)
		return nullptr;

	// Rasterize at twice the requested size and scale down, so the result is
	// supersampled rather than upscaled.
	Ref< drawing::Image > meshThumb = new drawing::Image(
		drawing::PixelFormat::getR8G8B8A8(),
		size * 2,
		size * 2
	);
	meshThumb->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	MeshAssetRasterizer().generate(editor, asset, meshThumb);

	drawing::ScaleFilter scaleFilter(
		size,
		size,
		drawing::ScaleFilter::MnAverage,
		drawing::ScaleFilter::MgLinear
	);
	meshThumb->apply(&scaleFilter);

	Ref< drawing::Image > shadow = meshThumb->clone();

	drawing::GaussianBlurFilter blurFilter(size / 4);
	shadow->apply(&blurFilter);

	const float half = size / 2.0f;
	for (int32_t y = 0; y < size; ++y)
	{
		const float vy = min(4.0f * min(y, size - 1 - y) / half, 1.0f);

		for (int32_t x = 0; x < size; ++x)
		{
			const float vx = min(4.0f * min(x, size - 1 - x) / half, 1.0f);
			const Scalar vignette(min(vx, vy));

			Color4f alpha;
			shadow->getPixelUnsafe(x, y, alpha);

			Color4f color;
			meshThumb->getPixelUnsafe(x, y, color);
			
			color.setAlpha(max(color.getAlpha(), alpha.getAlpha() * 0.5_simd * vignette));
			meshThumb->setPixelUnsafe(x, y, color);
		}
	}

	return new ui::Bitmap(meshThumb);
}

}
