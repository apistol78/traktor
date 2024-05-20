/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <functional>
#include "Core/Io/FileSystem.h"
#include "Core/Math/Const.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Editor/IEditor.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/ModelRasterizer.h"
#include "Model/Operations/Triangulate.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/Editor/MeshAssetRasterizer.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureSet.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.MeshAssetRasterizer", MeshAssetRasterizer, Object)

bool MeshAssetRasterizer::generate(const editor::IEditor* editor, const MeshAsset* asset, drawing::Image* outImage) const
{
	const std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	const std::wstring modelCachePath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");

	const Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());

	Ref< model::Model > model = model::ModelCache::getInstance().getMutable(modelCachePath, fileName, asset->getImportFilter());
	if (!model)
		return false;

	if (!model::Triangulate().apply(*model))
		return false;

	// Rasterize model.
	const Aabb3 boundingBox = model->getBoundingBox();
	const Scalar maxExtent = (boundingBox.getExtent() * Vector4(1.0f, 1.0f, 0.0f, 0.0f)).max();
	const Scalar invMaxExtent = 1.0_simd / maxExtent;
	const Matrix44 modelView = translate(0.0f, 0.0f, 2.5f) * scale(invMaxExtent, invMaxExtent, invMaxExtent) * rotateY(/*asset->getPreviewAngle()*/3.0f) * translate(-boundingBox.getCenter());
	return model::ModelRasterizer().generate(model, modelView, outImage);
}

}
