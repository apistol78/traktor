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

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshAssetRasterizer", MeshAssetRasterizer, Object)

bool MeshAssetRasterizer::generate(const editor::IEditor* editor, const MeshAsset* asset, drawing::Image* outImage) const
{
	Ref< model::Model > model = prepareModel(editor, asset);
	if (!model)
		return false;

	// Rasterize model.
	const Aabb3 boundingBox = model->getBoundingBox();
	const Scalar maxExtent = (boundingBox.getExtent() * Vector4(1.0f, 1.0f, 0.0f, 0.0f)).max();
	const Scalar invMaxExtent = 1.0_simd / maxExtent;
	const Matrix44 modelView = translate(0.0f, 0.0f, 2.5f) * scale(invMaxExtent, invMaxExtent, invMaxExtent) * rotateY(asset->getPreviewAngle()) * translate(-boundingBox.getCenter());
	return model::ModelRasterizer().generate(model, modelView, outImage);
}

bool MeshAssetRasterizer::generate(const editor::IEditor* editor, const MeshAsset* asset, float yaw, float pitch, drawing::Image* outImage) const
{
	Ref< model::Model > model = prepareModel(editor, asset);
	if (!model)
		return false;

	// Frame using the largest extent across all three axes so the model stays
	// inside the view at any yaw/pitch (the single-angle path above only turns
	// about Y, so it can frame on the X/Y silhouette alone).
	const Aabb3 boundingBox = model->getBoundingBox();
	const Scalar maxExtent = (boundingBox.getExtent() * Vector4(1.0f, 1.0f, 1.0f, 0.0f)).max();
	const Scalar invMaxExtent = 1.0_simd / maxExtent;
	// Negate pitch so a positive pitch tilts the camera to look DOWN onto the
	// model's top (+Y). With +pitch the model tips toward the viewer's underside,
	// and single-sided geometry (e.g. floor tiles with up-facing triangles only)
	// is backface-culled, leaving the view nearly empty.
	const Matrix44 modelView = translate(0.0f, 0.0f, 2.5f) * scale(invMaxExtent, invMaxExtent, invMaxExtent) * rotateX(-pitch) * rotateY(yaw) * translate(-boundingBox.getCenter());
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
