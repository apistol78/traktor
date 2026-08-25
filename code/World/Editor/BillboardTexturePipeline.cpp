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
#include "Database/Instance.h"
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

#include <cmath>

namespace traktor::world
{
namespace
{

/*! Directory debug atlases are written into. */
const wchar_t c_debugOutputPath[] = L"data/Temp/Billboard";

/*! Encoding of a normal pointing straight back at the view, i.e. (0, 0, -1). */
const Color4f c_clearNormal(0.5f, 0.5f, 0.0f, 0.0f);

void saveDebugImage(const drawing::Image* image, const std::wstring& name)
{
	const Path fileName = std::wstring(c_debugOutputPath) + L"/" + name + L".png";
	if (image->save(fileName))
		log::info << L"Billboard atlas written to \"" << fileName.getPathName() << L"\"." << Endl;
	else
		log::warning << L"Unable to write billboard atlas to \"" << fileName.getPathName() << L"\"." << Endl;
}

/*! Smallest distance from the camera to the model centre, in normalized model extents.
 *
 * The model is scaled so the framed square become a unit square, and is then pushed this
 * far away; the actual distance grow with the model's normalized radius so the entire
 * model stay inside the [0, 2 * distance] depth range of the orthographic frustum, which
 * a tall entity seen from above would otherwise fall out of.
 */
const float c_minCameraDistance = 2.0f;

struct SourceModel
{
	Ref< const model::Model > model;
	Transform transform;
};

typedef SmallMap< const TypeInfo*, Ref< const IEntityReplicator > > entity_replicators_t;

/*! Load and attach the images of every material which only reference its texture by guid.
 *
 * Replicators bind texture guids rather than images, since the pipelines they feed
 * normally resolve textures themselves; the software rasterizer however need the
 * actual pixels to shade with.
 */
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

				if (image != nullptr && textureAsset->m_output.m_assumeLinearGamma)
				{
					// Rasterizer expect gamma color space; convert as the mesh preview do.
					const drawing::GammaFilter gammaFilter(1.0f, 2.2f);
					image->apply(&gammaFilter);
				}
			}
			else
				log::warning << L"Unable to open billboard source texture \"" << filePath.getPathName() << L"\"." << Endl;
		}

		// Cache the result, null included, so a missing texture is only reported once.
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
		const IEntityReplicator* entityReplicator = entityReplicators[&type_of(componentData)];
		if (entityReplicator == nullptr)
			continue;

		Ref< model::Model > model = entityReplicator->createModel(pipelineCommon, entityData, componentData, IEntityReplicator::Usage::Visual);
		if (model)
			return model;
	}
	return nullptr;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.BillboardTexturePipeline", 1, BillboardTexturePipeline, editor::DefaultPipeline)

bool BillboardTexturePipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	if (!editor::DefaultPipeline::create(settings, database))
		return false;

	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");

	// Create entity replicators.
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
	pipelineDepends->addDependency(asset->m_entity, editor::PdfUse);

	Ref< const ISerializable > sourceData = pipelineDepends->getObjectReadOnly(asset->m_entity);
	if (!sourceData)
		return true;

	// The atlas is rasterized from the entity's geometry, so everything the entity
	// reference is an input to this build. Component pipelines declare their assets
	// PdfBuild only -- which is right for them, as each asset build into an instance of
	// its own -- but the global hash deciding whether to rebuild only recurse through
	// PdfUse edges, so without repeating those references here the atlas would never be
	// rebuilt when a mesh or one of its textures change.
	//
	// Resolved the same way buildOutput does, so the two agree on what was consumed.
	AlignedVector< Guid > externalEntities;
	Ref< ISerializable > resolvedData = resolveExternal(
		[&](const Guid& objectId) -> Ref< const ISerializable > {
		return pipelineDepends->getObjectReadOnly(objectId);
	},
		sourceData,
		Guid::null,
		&externalEntities);

	for (const auto& externalEntity : externalEntities)
		pipelineDepends->addDependency(externalEntity, editor::PdfUse);

	const EntityData* rootEntityData = dynamic_type_cast< const EntityData* >(resolvedData);
	if (!rootEntityData)
		return true;

	const auto addComponentDependencies = [&](const EntityData* entityData) {
		for (auto componentData : entityData->getComponents())
		{
			const IEntityReplicator* entityReplicator = m_entityReplicators[&type_of(componentData)];
			if (entityReplicator == nullptr)
				continue;

			for (auto dependentComponent : entityReplicator->getDependentComponents(entityData, componentData))
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

	if (angles < 1 || elevations < 1 || tileSize < 1)
	{
		log::error << L"Billboard texture pipeline failed; angles, elevations and tile size must all be at least one." << Endl;
		return false;
	}

	if (asset->m_entity.isNull())
	{
		log::error << L"Billboard texture pipeline failed; no entity referenced." << Endl;
		return false;
	}

	Ref< const ISerializable > sourceData = pipelineBuilder->getObjectReadOnly(asset->m_entity);
	if (!sourceData)
	{
		log::error << L"Billboard texture pipeline failed; unable to read entity " << asset->m_entity.format() << L"." << Endl;
		return false;
	}

	// Flatten external references so replicators are handed the components themselves.
	Ref< ISerializable > resolvedData = resolveExternal(
		[&](const Guid& objectId) -> Ref< const ISerializable > {
		return pipelineBuilder->getObjectReadOnly(objectId);
	},
		sourceData,
		Guid::null,
		nullptr);

	const EntityData* rootEntityData = dynamic_type_cast< const EntityData* >(resolvedData);
	if (!rootEntityData)
	{
		log::error << L"Billboard texture pipeline failed; " << asset->m_entity.format() << L" is not an entity." << Endl;
		return false;
	}

	// Child entities carry absolute transforms, so bring every entity back into the
	// root's own space; the atlas should be centered on the entity, not on the world.
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
		// Hidden entities are not part of what the billboard stand in for.
		if (!entityData->getState().visible)
			return Traverser::Result::Skip;

		collectEntityModel(entityData);
		return Traverser::Result::Continue;
	});

	// A single model let the rasterizer resolve visibility between the entities with
	// one depth buffer, which separate passes could not do.
	Ref< model::Model > mergedModel = new model::Model();
	for (const auto& sourceModel : sourceModels)
	{
		const model::MergeModel mrg(*sourceModel.model, sourceModel.transform, 0.0001f);
		mergedModel->apply(mrg);
	}

	if (mergedModel->getPolygonCount() == 0)
	{
		log::error << L"Billboard texture pipeline failed; entity " << asset->m_entity.format() << L" contain no geometry." << Endl;
		return false;
	}

	resolveMaterialImages(pipelineBuilder, m_assetPath, mergedModel);

	const Aabb3 boundingBox = mergedModel->getBoundingBox();
	if (boundingBox.empty())
	{
		log::error << L"Billboard texture pipeline failed; entity " << asset->m_entity.format() << L" has an empty bounding box." << Endl;
		return false;
	}

	const Vector4 extent = boundingBox.getExtent();

	// Rotation about Y never change how wide the entity project, so the horizontal
	// circumradius bound every column; rotation about X trade height for depth, thus the
	// tallest elevation bound every row. Feeding both into one square frustum keep the
	// world size of a tile identical across the atlas, which is what let a billboard
	// component pick a tile from the view angle alone.
	const Scalar radiusXZ = (extent * Vector4(1.0f, 0.0f, 1.0f, 0.0f)).length();
	Scalar halfHeight = 0.0_simd;
	for (int32_t i = 0; i < elevations; ++i)
	{
		const float elevation = (elevations > 1) ? asset->m_elevationFrom + (asset->m_elevationTo - asset->m_elevationFrom) * i / (elevations - 1) : asset->m_elevationFrom;
		halfHeight = max(halfHeight, extent.y() * Scalar(std::abs(std::cos(elevation))) + radiusXZ * Scalar(std::abs(std::sin(elevation))));
	}

	const Scalar halfExtent = max(radiusXZ, halfHeight);
	if (halfExtent <= 0.0_simd)
	{
		log::error << L"Billboard texture pipeline failed; entity " << asset->m_entity.format() << L" is degenerate." << Endl;
		return false;
	}
	const Scalar invExtent = 1.0_simd / halfExtent;

	// Empty normal texels decode to a normal pointing straight back at the view, which is
	// the least surprising thing for filtering to drag in along the silhouette.
	const bool normals = (asset->m_content == BillboardTextureAsset::Content::Normals);
	const Color4f clearColor = normals ? c_clearNormal : Color4f(0.0f, 0.0f, 0.0f, 0.0f);

	Ref< drawing::Image > atlas = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), tileSize * angles, tileSize * elevations);
	atlas->clear(clearColor);

	Ref< drawing::Image > albedoTile = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), tileSize, tileSize);
	Ref< drawing::Image > normalTile = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), tileSize, tileSize);

	model::ModelRasterizer::Options options;
	options.alphaThreshold = asset->m_alphaThreshold;
	options.writeAlpha = true;
	options.twoSided = asset->m_twoSided;
	// Bake what the entity is, not how it happen to be lit; the shader sampling the
	// atlas do its own lighting from the baked normals.
	options.unlit = true;

	// Framing only bound how far the model reach sideways, never in depth, so derive the
	// camera distance from the bounding sphere instead and keep every fragment in range.
	const float normalizedRadius = (float)(extent.length() * invExtent);
	const float cameraDistance = (normalizedRadius + 1.0f > c_minCameraDistance) ? (normalizedRadius + 1.0f) : c_minCameraDistance;

	const Matrix44 projection = orthoLh(2.0f, 2.0f, 0.0f, 2.0f * cameraDistance);

	for (int32_t ey = 0; ey < elevations; ++ey)
	{
		const float elevation = (elevations > 1) ? asset->m_elevationFrom + (asset->m_elevationTo - asset->m_elevationFrom) * ey / (elevations - 1) : asset->m_elevationFrom;
		for (int32_t ex = 0; ex < angles; ++ex)
		{
			const float yaw = TWO_PI * ex / angles;

			// Negate elevation so a positive angle lift the camera and look down onto the
			// entity, matching how the mesh preview interpret its pitch.
			const Matrix44 modelView =
				translate(0.0f, 0.0f, cameraDistance) *
				scale(invExtent, invExtent, invExtent) *
				rotateX(-elevation) *
				rotateY(yaw) *
				translate(-boundingBox.getCenter());

			albedoTile->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
			normalTile->clear(c_clearNormal);

			if (!model::ModelRasterizer().generate(mergedModel, modelView, projection, options, albedoTile, normalTile))
			{
				log::error << L"Billboard texture pipeline failed; unable to rasterize entity " << asset->m_entity.format() << L"." << Endl;
				return false;
			}

			// Both maps come out of one pass; only the one this asset bake is kept.
			atlas->copy(normals ? normalTile : albedoTile, ex * tileSize, ey * tileSize, 0, 0, tileSize, tileSize);
		}
	}

	// A billboard component need to be told the same framing to place its quad; log it
	// in the terms the component's properties are expressed in.
	const Vector4 center = boundingBox.getCenter();
	log::info << L"Billboard " << (normals ? L"normal" : L"albedo") << L" atlas " << atlas->getWidth() << L"x" << atlas->getHeight() << L", " << angles << L" angle(s) by " << elevations << L" elevation(s)." << Endl;
	log::info << L"\tsize " << (2.0f * (float)halfExtent) << L", offset " << (float)center.x() << L", " << (float)center.y() << L", " << (float)center.z() << L"." << Endl;

	// Dump the atlas as a plain image; the built texture is compressed and mipped which
	// make it awkward to inspect when a billboard end up looking wrong.
	const std::wstring debugName = (sourceInstance != nullptr) ? sourceInstance->getName() : outputGuid.format();
	if (FileSystem::getInstance().makeAllDirectories(Path(c_debugOutputPath)))
		saveDebugImage(atlas, debugName);
	else
		log::warning << L"Unable to create billboard debug directory \"" << c_debugOutputPath << L"\"." << Endl;

	Ref< render::TextureOutput > output = new render::TextureOutput();
	output->m_textureFormat = render::TfInvalid;
	output->m_generateMips = asset->m_generateMips;
	output->m_keepZeroAlpha = false;
	output->m_textureType = render::Tt2D;
	output->m_hasAlpha = true;
	output->m_enableCompression = asset->m_enableCompression;
	// Encoded normals are data, not color, so no gamma conversion may touch them.
	output->m_assumeLinearGamma = normals;
	// Bleed into the cut out texels, otherwise filtering pull the cleared background in
	// along the silhouette, and keep the cut out from eroding away in the mip chain.
	output->m_dilateImage = true;
	output->m_preserveAlphaCoverage = (!normals && asset->m_alphaThreshold > 0.0f);
	output->m_alphaCoverageReference = asset->m_alphaThreshold;

	return pipelineBuilder->buildAdHocOutput(
		output,
		outputPath,
		outputGuid,
		atlas);
}

}
