/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Editor/BillboardTexturePipeline.h"

#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmPrimitive.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Settings/PropertyString.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Model.h"
#include "Model/ModelRasterizer.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/Triangulate.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "World/Editor/BillboardTextureAsset.h"
#include "World/Editor/IEntityReplicator.h"
#include "World/Editor/ResolveExternal.h"
#include "World/Editor/Traverser.h"
#include "World/EntityData.h"
#include "World/IEntityComponentData.h"

#include <algorithm>
#include <cmath>

namespace traktor::world
{
namespace
{

/*! Encoding of a normal pointing straight back at the view, i.e. (0, 0, -1). */
const Color4f c_clearNormal(0.5f, 0.5f, 0.0f, 0.0f);

/*! Smallest distance from camera to framed centre, in normalized model extents. */
const float c_minCameraDistance = 2.0f;

/*! Discard fragments below this alpha, cut out alpha mapped foliage. */
const float c_alphaThreshold = 0.75f;

struct SourceModel
{
	Ref< const model::Model > model;
	Transform transform;
};

typedef SmallMap< const TypeInfo*, Ref< const IEntityReplicator > > entity_replicators_t;

/*! Load images of materials which only reference their texture by guid; the rasterizer need the pixels. */
void resolveMaterialImages(editor::IPipelineCommon* pipelineCommon, const std::wstring& assetPath, model::Model* model)
{
	SmallMap< Guid, Ref< drawing::Image > > images;
	for (auto& material : model->getMaterials())
	{
		model::Material::Map diffuseMap = material.getDiffuseMap();
		if (diffuseMap.image != nullptr || diffuseMap.texture.isNull())
			continue;

		const auto it = images.find(diffuseMap.texture);
		if (it != images.end())
		{
			diffuseMap.image = it->second;
			material.setDiffuseMap(diffuseMap);
			continue;
		}

		Ref< drawing::Image > image;
		Ref< const render::TextureAsset > textureAsset = pipelineCommon->getObjectReadOnly< render::TextureAsset >(diffuseMap.texture);
		if (textureAsset)
		{
			const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + textureAsset->getFileName());
			Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
			if (file)
			{
				image = drawing::Image::load(file, textureAsset->getFileName().getExtension());
				file->close();

				// Rasterizer expect gamma color space.
				if (image != nullptr && textureAsset->m_output.m_assumeLinearGamma)
				{
					const drawing::GammaFilter gammaFilter(1.0f, 2.2f);
					image->apply(&gammaFilter);
				}
			}
			else
				log::warning << L"Unable to open billboard source texture \"" << filePath.getPathName() << L"\"." << Endl;
		}

		// Cache result, null included, so a missing texture is only reported once.
		images[diffuseMap.texture] = image;

		diffuseMap.image = image;
		material.setDiffuseMap(diffuseMap);
	}
}

/*! Create a model replica of a single entity, or null if no component describe geometry. */
Ref< model::Model > createEntityModel(editor::IPipelineCommon* pipelineCommon, const entity_replicators_t& entityReplicators, const EntityData* entityData)
{
	for (auto componentData : entityData->getComponents())
	{
		const auto it = entityReplicators.find(&type_of(componentData));
		if (it == entityReplicators.end())
			continue;

		Ref< model::Model > model = it->second->createModel(pipelineCommon, entityData, componentData, IEntityReplicator::Usage::Visual, IEntityReplicator::Flags::Default);
		if (model)
			return model;
	}
	return nullptr;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.BillboardTexturePipeline", 3, BillboardTexturePipeline, editor::DefaultPipeline)

bool BillboardTexturePipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	if (!editor::DefaultPipeline::create(settings, database))
		return false;

	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");

	for (const auto& entityReplicatorType : type_of< IEntityReplicator >().findAllOf(false))
	{
		Ref< IEntityReplicator > entityReplicator = mandatory_non_null_type_cast< IEntityReplicator* >(entityReplicatorType->createInstance());
		if (!entityReplicator->create(settings))
			return false;

		for (auto supportedType : entityReplicator->getSupportedTypes())
			m_entityReplicators[supportedType] = entityReplicator;
	}

	return true;
}

TypeInfoSet BillboardTexturePipeline::getAssetTypes() const
{
	return makeTypeInfoSet< BillboardTextureAsset >();
}

bool BillboardTexturePipeline::shouldCache() const
{
	return true;
}

bool BillboardTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid) const
{
	const BillboardTextureAsset* asset = mandatory_non_null_type_cast< const BillboardTextureAsset* >(sourceAsset);

	pipelineDepends->addDependency< render::TextureOutput >();

	// Resolved the same way buildOutput does, so the two agree on what was consumed.
	AlignedVector< Guid > externalEntities;
	Ref< ISerializable > resolvedData = resolveExternal(
		[&](const Guid& objectId) -> Ref< const ISerializable > {
		return pipelineDepends->getObjectReadOnly(objectId);
	},
		asset->m_entityData,
		Guid::null,
		&externalEntities);

	for (const auto& externalEntity : externalEntities)
		pipelineDepends->addDependency(externalEntity, editor::PdfUse);

	const EntityData* rootEntityData = dynamic_type_cast< const EntityData* >(resolvedData);
	if (!rootEntityData)
		return true;

	// Assets the geometry is replicated from are used, not built, by the atlas; hash must include them.
	const auto addComponentDependencies = [&](const EntityData* entityData) {
		for (auto componentData : entityData->getComponents())
		{
			const auto it = m_entityReplicators.find(&type_of(componentData));
			if (it == m_entityReplicators.end())
				continue;

			for (auto dependentComponent : it->second->getDependentComponents(entityData, componentData))
			{
				Ref< Reflection > reflection = Reflection::create(dependentComponent);
				if (!reflection)
					continue;

				RefArray< ReflectionMember > guidMembers;
				reflection->findMembers(RfpMemberType(type_of< RfmPrimitiveGuid >()), guidMembers);
				for (auto guidMember : guidMembers)
				{
					const Guid& guid = mandatory_non_null_type_cast< const RfmPrimitiveGuid* >(guidMember.ptr())->get();
					if (guid.isNotNull())
						pipelineDepends->addDependency(guid, editor::PdfUse);
				}
			}
		}
	};

	addComponentDependencies(rootEntityData);
	Traverser::visit(rootEntityData, [&](const EntityData* entityData) -> Traverser::Result {
		addComponentDependencies(entityData);
		return Traverser::Result::Continue;
	});

	return true;
}

bool BillboardTexturePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason) const
{
	const BillboardTextureAsset* asset = mandatory_non_null_type_cast< const BillboardTextureAsset* >(sourceAsset);

	const int32_t angles = asset->m_angles;
	const int32_t elevations = asset->m_elevations;
	const int32_t tileSize = asset->m_tileSize;

	if (angles < 1 || elevations < 1 || tileSize < 1 || asset->m_size <= 0.0f)
	{
		log::error << L"Billboard texture pipeline failed; angles, elevations, tile size and size must all be positive." << Endl;
		return false;
	}

	// Flatten external references so replicators are handed the components themselves.
	Ref< ISerializable > resolvedData = resolveExternal(
		[&](const Guid& objectId) -> Ref< const ISerializable > {
		return pipelineBuilder->getObjectReadOnly(objectId);
	},
		asset->m_entityData,
		Guid::null,
		nullptr);

	const EntityData* rootEntityData = dynamic_type_cast< const EntityData* >(resolvedData);
	if (!rootEntityData)
	{
		log::error << L"Billboard texture pipeline failed; no entity." << Endl;
		return false;
	}

	// Child entities carry absolute transforms; bring every entity into the root's own space.
	const Transform invRootTransform = rootEntityData->getTransform().inverse();

	AlignedVector< SourceModel > sourceModels;
	const auto collectEntityModel = [&](const EntityData* entityData) {
		Ref< model::Model > model = createEntityModel(pipelineBuilder, m_entityReplicators, entityData);
		if (!model)
			return;

		model->apply(model::Triangulate());
		sourceModels.push_back(SourceModel{ model, invRootTransform * entityData->getTransform() });
	};

	collectEntityModel(rootEntityData);
	Traverser::visit(rootEntityData, [&](const EntityData* entityData) -> Traverser::Result {
		if (!entityData->getState().visible)
			return Traverser::Result::Skip;

		collectEntityModel(entityData);
		return Traverser::Result::Continue;
	});

	// Single model so the rasterizer resolve visibility between entities with one depth buffer.
	Ref< model::Model > mergedModel = new model::Model();
	for (const auto& sourceModel : sourceModels)
	{
		const model::MergeModel mrg(*sourceModel.model, sourceModel.transform, 0.0001f);
		mergedModel->apply(mrg);
	}

	if (mergedModel->getPolygonCount() == 0)
	{
		log::error << L"Billboard texture pipeline failed; entity \"" << rootEntityData->getName() << L"\" contain no geometry." << Endl;
		return false;
	}

	resolveMaterialImages(pipelineBuilder, m_assetPath, mergedModel);

	const Vector4 center = asset->m_offset.xyz1();
	const Scalar invExtent(2.0f / asset->m_size);

	// Camera is pushed back far enough for the model's bounding sphere to stay within depth range.
	const Aabb3 boundingBox = mergedModel->getBoundingBox();
	const Scalar radius = max((boundingBox.mx - center).absolute(), (boundingBox.mn - center).absolute()).xyz0().length();
	const float cameraDistance = std::max((float)(radius * invExtent) + 1.0f, c_minCameraDistance);

	const Matrix44 projection = orthoLh(2.0f, 2.0f, 0.0f, 2.0f * cameraDistance);

	// Empty normal texels decode to a normal facing the view; least surprising for filtering along silhouettes.
	const bool normals = (asset->m_content == BillboardTextureAsset::Content::Normals);
	const Color4f clearColor = normals ? c_clearNormal : Color4f(0.0f, 0.0f, 0.0f, 0.0f);

	Ref< drawing::Image > atlas = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), tileSize * angles, tileSize * elevations);
	atlas->clear(clearColor);

	Ref< drawing::Image > albedoTile = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), tileSize, tileSize);
	Ref< drawing::Image > normalTile = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), tileSize, tileSize);

	// Bake what the entity is, not how it happen to be lit; billboard shader light from baked normals.
	model::ModelRasterizer::Options options;
	options.alphaThreshold = c_alphaThreshold;
	options.writeAlpha = true;
	options.twoSided = true;
	options.unlit = true;

	for (int32_t ey = 0; ey < elevations; ++ey)
	{
		const float elevation = (elevations > 1) ? asset->m_elevationFrom + (asset->m_elevationTo - asset->m_elevationFrom) * ey / (elevations - 1) : asset->m_elevationFrom;
		for (int32_t ex = 0; ex < angles; ++ex)
		{
			const float yaw = TWO_PI * ex / angles;

			// Negated elevation; a positive angle lift the camera and look down onto the entity.
			const Matrix44 modelView =
				translate(0.0f, 0.0f, cameraDistance) *
				scale(invExtent, invExtent, invExtent) *
				rotateX(-elevation) *
				rotateY(yaw) *
				translate(-center);

			albedoTile->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
			normalTile->clear(c_clearNormal);

			if (!model::ModelRasterizer().generate(mergedModel, modelView, projection, options, albedoTile, normalTile))
			{
				log::error << L"Billboard texture pipeline failed; unable to rasterize entity \"" << rootEntityData->getName() << L"\"." << Endl;
				return false;
			}

			atlas->copy(normals ? normalTile : albedoTile, ex * tileSize, ey * tileSize, 0, 0, tileSize, tileSize);
		}
	}

	Ref< render::TextureOutput > output = new render::TextureOutput();
	output->m_textureFormat = render::TfInvalid;
	output->m_generateMips = true;
	output->m_keepZeroAlpha = false;
	output->m_textureType = render::Tt2D;
	output->m_hasAlpha = true;
	output->m_enableCompression = true;
	output->m_assumeLinearGamma = normals;
	output->m_dilateImage = true;
	output->m_preserveAlphaCoverage = !normals;
	output->m_alphaCoverageReference = c_alphaThreshold;

	return pipelineBuilder->buildAdHocOutput(
		output,
		outputPath,
		outputGuid,
		atlas);
}

}
