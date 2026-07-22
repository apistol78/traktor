/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshAssetRasterizer.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphPreview.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureSet.h"

namespace traktor::mesh
{
	namespace
	{

int32_t wrap(int32_t v, int32_t l)
{
	const int32_t c = v % l;
	return (c < 0) ? c + l : c;
}

/*! Load the asset's source model from the model cache (no triangulation or
 * material binding). Cheap path for callers that only need geometry, e.g. the
 * bounding box. Returns null if the model file cannot be read. */
Ref< model::Model > loadModel(const editor::IEditor* editor, const MeshAsset* asset)
{
	const std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	const std::wstring modelCachePath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");

	const Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());
	return model::ModelCache::getInstance().getMutable(modelCachePath, fileName, asset->getImportFilter());
}

/*! Load the asset's model and bind its material maps (shader-graph previews and
 * texture images) ready for rasterization. Shared by both generate() overloads.
 * Returns null if the model file cannot be read. */
Ref< model::Model > prepareModel(const editor::IEditor* editor, const MeshAsset* asset)
{
	Ref< model::Model > model = loadModel(editor, asset);
	if (!model)
		return nullptr;

	if (!model->apply(model::Triangulate()))
		return nullptr;

	const std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	const auto& materialTextures = asset->getMaterialTextures();
	const auto& materialShaders = asset->getMaterialShaders();

	// Bind texture references in material maps.
	for (auto& material : model->getMaterials())
	{
		auto diffuseMap = material.getDiffuseMap();
		auto it = materialTextures.find(diffuseMap.name);
		if (it != materialTextures.end())
		{
			diffuseMap.texture = it->second;
			material.setDiffuseMap(diffuseMap);
		}
	}

	// Load texture images and attach to materials.
	SmallMap< Path, Ref< drawing::Image > > images;
	for (auto& material : model->getMaterials())
	{
		model::Material::Map& diffuseMap = material.getDiffuseMap();

		const auto it = materialShaders.find(material.getName());
		if (it != materialShaders.end())
		{
			const Ref< render::ShaderGraph > materialShaderGraph = editor->getSourceDatabase()->getObjectReadOnly< render::ShaderGraph >(it->second);
			if (!materialShaderGraph)
				continue;

			Ref< drawing::Image > image = render::ShaderGraphPreview(editor).generate(materialShaderGraph, 128, 128);
			if (!image)
				continue;


			diffuseMap.image = image;
		}

		if (diffuseMap.image == nullptr && diffuseMap.texture.isNotNull())
		{
			Ref< const render::TextureAsset > textureAsset = editor->getSourceDatabase()->getObjectReadOnly< render::TextureAsset >(diffuseMap.texture);
			if (!textureAsset)
				continue;

			const Path filePath = FileSystem::getInstance().getAbsolutePath(assetPath, textureAsset->getFileName());
			Ref< drawing::Image > image = images[filePath];
			if (image == nullptr)
			{
				Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
				if (file)
				{
					image = drawing::Image::load(file, textureAsset->getFileName().getExtension());
					if (image && textureAsset->m_output.m_assumeLinearGamma)
					{
						// Convert to gamma color space.
						const drawing::GammaFilter gammaFilter(1.0f, 2.2f);
						image->apply(&gammaFilter);
					}
					images[filePath] = image;
				}
			}

			diffuseMap.image = image;
		}

		material.setDiffuseMap(diffuseMap);
	}

	return model;
}

/*! Distance from the camera to the model centre.
 *
 * The model is normalized so its bounding sphere has unit radius (see
 * calculateModelView). ModelRasterizer projects with a 70-degree FOV and a 0.1
 * near plane, so at this distance a unit sphere is fully visible
 * (asin(1/2.5) ~= 24 deg < 35 deg half-FOV) and its nearest point (z = 1.5)
 * never crosses the near plane. */
const float c_cameraDistance = 2.0f;

/*! Build the model->view matrix that frames the model for ModelRasterizer.
 *
 * Normalizing by the bounding-sphere radius (rather than a per-axis extent)
 * keeps the framing rotation invariant: the model stays fully inside the view
 * at any orientation and never clips the near plane. 'rotation' is the
 * orientation applied to the recentered model. */
Matrix44 calculateModelView(const Aabb3& boundingBox, const Matrix44& rotation)
{
	const Scalar radius = boundingBox.getExtent().length();
	const Scalar invRadius = (radius > FUZZY_EPSILON) ? (1.0_simd / radius) : 1.0_simd;
	return
		translate(0.0f, 0.0f, c_cameraDistance) *
		scale(invRadius, invRadius, invRadius) *
		rotation *
		translate(-boundingBox.getCenter());
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshAssetRasterizer", MeshAssetRasterizer, Object)

bool MeshAssetRasterizer::generate(const editor::IEditor* editor, const MeshAsset* asset, drawing::Image* outImage) const
{
	Ref< model::Model > model = prepareModel(editor, asset);
	if (!model)
		return false;

	// Rasterize model.
	const Matrix44 modelView = calculateModelView(model->getBoundingBox(), rotateY(asset->getPreviewAngle()));
	return model::ModelRasterizer().generate(model, modelView, outImage);
}

bool MeshAssetRasterizer::generate(const editor::IEditor* editor, const MeshAsset* asset, float yaw, float pitch, drawing::Image* outImage) const
{
	Ref< model::Model > model = prepareModel(editor, asset);
	if (!model)
		return false;

	// Negate pitch so a positive pitch tilts the camera to look DOWN onto the
	// model's top (+Y). With +pitch the model tips toward the viewer's underside,
	// and single-sided geometry (e.g. floor tiles with up-facing triangles only)
	// is backface-culled, leaving the view nearly empty.
	const Matrix44 modelView = calculateModelView(model->getBoundingBox(), rotateX(-pitch) * rotateY(yaw));
	return model::ModelRasterizer().generate(model, modelView, outImage);
}

bool MeshAssetRasterizer::getBoundingBox(const editor::IEditor* editor, const MeshAsset* asset, Aabb3& outBoundingBox) const
{
	Ref< model::Model > model = loadModel(editor, asset);
	if (!model)
		return false;

	outBoundingBox = model->getBoundingBox();
	return !outBoundingBox.empty();
}

}
