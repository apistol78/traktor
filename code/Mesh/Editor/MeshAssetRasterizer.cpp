/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

namespace traktor
{
	namespace mesh
	{
		namespace
		{

int32_t wrap(int32_t v, int32_t l)
{
	const int32_t c = v % l;
	return (c < 0) ? c + l : c;
}

		}


T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshAssetRasterizer", MeshAssetRasterizer, Object)

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
		const auto it = materialShaders.find(material.getName());
		if (it != materialShaders.end())
		{
			const Ref< render::ShaderGraph > materialShaderGraph = editor->getSourceDatabase()->getObjectReadOnly< render::ShaderGraph >(it->second);
			if (!materialShaderGraph)
				continue;
		
			Ref< drawing::Image > image = render::ShaderGraphPreview(editor).generate(materialShaderGraph, 128, 128);
			if (!image)
				continue;

			auto diffuseMap = material.getDiffuseMap();
			diffuseMap.image = image;			
			material.setDiffuseMap(diffuseMap);
		}

		//auto diffuseMap = material.getDiffuseMap();
		//if (diffuseMap.texture.isNotNull())
		//{
		//	Ref< const render::TextureAsset > textureAsset = editor->getSourceDatabase()->getObjectReadOnly< render::TextureAsset >(diffuseMap.texture);
		//	if (!textureAsset)
		//		continue;

		//	const Path filePath = FileSystem::getInstance().getAbsolutePath(assetPath, textureAsset->getFileName());
		//	Ref< drawing::Image > image = images[filePath];
		//	if (image == nullptr)
		//	{
		//		Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
		//		if (file)
		//		{
		//			image = drawing::Image::load(file, textureAsset->getFileName().getExtension());
		//			if (image && textureAsset->m_output.m_assumeLinearGamma)
		//			{
		//				// Convert to gamma color space.
		//				const drawing::GammaFilter gammaFilter(1.0f, 2.2f);
		//				image->apply(&gammaFilter);							
		//			}
		//			images[filePath] = image;
		//		}
		//	}

		//	diffuseMap.image = image;			
		//	material.setDiffuseMap(diffuseMap);
		//}
	}

	// Rasterize model.
	const Aabb3 boundingBox = model->getBoundingBox();
	const Scalar maxExtent = (boundingBox.getExtent() * Vector4(1.0f, 1.0f, 0.0f, 0.0f)).max();
	const Scalar invMaxExtent = 1.0_simd / maxExtent;
	const Matrix44 modelView = translate(0.0f, 0.0f, 2.5f) * scale(invMaxExtent, invMaxExtent, invMaxExtent) * rotateY(asset->getPreviewAngle()) * translate(-boundingBox.getCenter());
	return model::ModelRasterizer().generate(model, modelView, outImage);
}

	}
}
