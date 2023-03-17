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
#include "Core/Math/Const.h"
#include "Core/Math/Triangle.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Editor/IEditor.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/Operations/Triangulate.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshAssetRasterizer.h"
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

	Ref< model::Model > model = model::ModelCache(modelCachePath).get(fileName, asset->getImportFilter());
	if (!model)
		return false;

	if (!model::Triangulate().apply(*model))
		return false;

	// Create list of texture references.
	std::map< std::wstring, Guid > materialTextures;

	// First use textures from texture set.
	const auto& textureSetId = asset->getTextureSet();
	if (textureSetId.isNotNull())
	{
		Ref< const render::TextureSet > textureSet = editor->getSourceDatabase()->getObjectReadOnly< render::TextureSet >(textureSetId);
		if (!textureSet)
			return false;

		materialTextures = textureSet->get();
	}

	// Then let explicit material textures override those from a texture set.
	for (const auto& mt : asset->getMaterialTextures())
		materialTextures[mt.first] = mt.second;

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
		auto diffuseMap = material.getDiffuseMap();
		if (diffuseMap.texture.isNotNull())
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
					if (image && textureAsset->m_output.m_linearGamma)
					{
						// Convert to gamma color space.
						drawing::GammaFilter gammaFilter(2.2f);
						image->apply(&gammaFilter);							
					}
					images[filePath] = image;
				}
			}

			diffuseMap.image = image;			
			material.setDiffuseMap(diffuseMap);
		}
	}

	const float hw = (float)(outImage->getWidth() / 2.0f);
	const float hh = (float)(outImage->getHeight() / 2.0f);
	const float r = (float)outImage->getWidth() / outImage->getHeight();

	const Aabb3 boundingBox = model->getBoundingBox();
	const Scalar maxExtent = (boundingBox.getExtent() * Vector4(1.0f, 1.0f, 0.0f, 0.0f)).max();
	const Scalar invMaxExtent = 1.0_simd / maxExtent;

	const Matrix44 modelView = translate(0.0f, 0.0f, 3.0f) * scale(invMaxExtent, invMaxExtent, invMaxExtent) * rotateY(asset->getPreviewAngle()) * translate(-boundingBox.getCenter());
	const Matrix44 projection = perspectiveLh(deg2rad(70.0f), r, 0.1f, 100.0f);

	const auto& materials = model->getMaterials();
	const auto& polygons = model->getPolygons();
	const auto& vertices = model->getVertices();
	const auto& positions = model->getPositions();
	const auto& normals = model->getNormals();
	const auto& texCoords = model->getTexCoords();

	AlignedVector< float > zbuffer(outImage->getWidth() * outImage->getHeight(), 1.0f);

	for (const auto& polygon : polygons)
	{
		if (polygon.getMaterial() == model::c_InvalidIndex)
			continue;

		const auto& polygonVertices = polygon.getVertices();
		if (polygonVertices.size() != 3)
			continue;

		Vector4 cp[3];
		Vector4 nm[3];
		Vector2 sp[3];
		Vector2 uv[3];

		for (size_t i = 0; i < 3; ++i)
		{
			if (polygonVertices[i] >= vertices.size())
				return false;

			const model::Vertex& vertex = vertices[polygonVertices[i]];
			if (vertex.getPosition() == model::c_InvalidIndex || vertex.getNormal() == model::c_InvalidIndex)
				return false;

			const Vector4& position = positions[vertex.getPosition()];
			const Vector4& normal = normals[vertex.getNormal()];

			Vector4 vp = modelView * position;
			cp[i] = projection * vp;
			cp[i] = cp[i] / cp[i].w();

			nm[i] = (modelView * normal).normalized();

			sp[i] = Vector2(
				cp[i].x() * hw + hw,
				hh - (cp[i].y() * hh)
			);

			if (vertex.getTexCoord(0) != model::c_InvalidIndex)
				uv[i] = texCoords[vertex.getTexCoord(0)];
			else
				uv[i].set(0.0f, 0.0f);			
		}

		// Discard too large triangles, to prevent broken meshes to halt editor.
		const float bw = std::max< float >({ sp[0].x, sp[1].x, sp[2].x }) - std::min< float >({ sp[0].x, sp[1].x, sp[2].x });
		const float bh = std::max< float >({ sp[0].y, sp[1].y, sp[2].y }) - std::min< float >({ sp[0].y, sp[1].y, sp[2].y });
		if (bw > 2.0f * hw || bh > 2.0f * hh)
			continue;

		const auto& polygonMaterial = materials[polygon.getMaterial()];
		const drawing::Image* texture = polygonMaterial.getDiffuseMap().image;
		if (texture != nullptr)
		{
			const int32_t tw = texture->getWidth();
			const int32_t th = texture->getHeight();

			triangle(sp[0], sp[1], sp[2], [&, tw, th](int32_t x, int32_t y, float alpha, float beta, float gamma) {
				if (x < 0 || x >= outImage->getWidth() || y < 0 || y >= outImage->getHeight())
					return;

				const int32_t offset = x + y * outImage->getWidth();

				const float z = cp[0].z() * Scalar(alpha) + cp[1].z() * Scalar(beta) + cp[2].z() * Scalar(gamma);
				if (z < zbuffer[offset])
				{
					Color4f color;
					Vector2 tc = uv[0] * alpha + uv[1] * beta + uv[2] * gamma;

					const int32_t tu = wrap((int32_t)(tc.x * tw), tw);
					const int32_t tv = wrap((int32_t)(tc.y * th), th);

					tc *= Vector2(tw, th);
					texture->getPixel(tu, tv, color);

					const Vector4 n = (nm[0] * Scalar(alpha) + nm[1] * Scalar(beta) + nm[2] * Scalar(gamma)).normalized();
					const Scalar d = -n.z();

					outImage->setPixelUnsafe(x, y, (color * d).rgb1());
					zbuffer[offset] = z;
				}
				});
		}
		else
		{
			triangle(sp[0], sp[1], sp[2], [&](int32_t x, int32_t y, float alpha, float beta, float gamma) {
				if (x < 0 || x >= outImage->getWidth() || y < 0 || y >= outImage->getHeight())
					return;

				const int32_t offset = x + y * outImage->getWidth();

				const float z = cp[0].z() * Scalar(alpha) + cp[1].z() * Scalar(beta) + cp[2].z() * Scalar(gamma);
				if (z < zbuffer[offset])
				{
					const Color4f& color = polygonMaterial.getColor();

					const Vector4 n = (nm[0] * Scalar(alpha) + nm[1] * Scalar(beta) + nm[2] * Scalar(gamma)).normalized();
					const Scalar d = -n.z();

					outImage->setPixelUnsafe(x, y, (color * d).rgb1());
					zbuffer[offset] = z;
				}
			});
		}
	}

	return true;
}

	}
}
