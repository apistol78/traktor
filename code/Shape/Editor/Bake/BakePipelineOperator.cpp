/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include <functional>
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/Range.h"
#include "Core/Math/Winding3.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Timer/Timer.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/CubeMap.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Filters/TransformFilter.h"
#include "Editor/DataAccessCache.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Editor/Pipeline/PipelineProfiler.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/UnwrapUV.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Render/Types.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Resource/AliasTextureResource.h"
#include "World/Editor/IEntityReplicator.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/Traverser.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/BakePipelineOperator.h"
#include "Shape/Editor/Bake/IblProbe.h"
#include "Shape/Editor/Bake/TracerEnvironment.h"
#include "Shape/Editor/Bake/TracerIrradiance.h"
#include "Shape/Editor/Bake/TracerLight.h"
#include "Shape/Editor/Bake/TracerModel.h"
#include "Shape/Editor/Bake/TracerOutput.h"
#include "Shape/Editor/Bake/TracerProcessor.h"
#include "Shape/Editor/Bake/TracerTask.h"
#include "World/EntityData.h"
#include "World/IrradianceGridResource.h"
#include "World/WorldRenderSettings.h"
#include "World/Editor/EditorAttributesComponentData.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Editor/ResolveExternal.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/GroupComponentData.h"
#include "World/Entity/LightComponentData.h"
#include "World/Entity/VolumeComponentData.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

const Guid c_lightmapProxyId(L"{A5F6E00A-6291-D640-825C-99006197AF49}");
const Guid c_lightmapDiffuseIdSeed(L"{A5A16214-0A01-4D6D-A509-6A5A16ACB6A3}");
const Guid c_lightmapDirectionalIdSeed(L"{BBCB9EFD-F519-49BE-A47A-66B7F1F0F5D1}");
const Guid c_outputIdSeed(L"{043B98C3-F93B-4510-8B73-1B5EEF2323E5}");
const Guid c_shapeMeshAssetSeed(L"{FEC54BB1-1F55-48F5-AB87-58FE1712C42D}");

/*! Log entity hierarchy. */
void describeEntity(OutputStream& os, const world::EntityData* entityData)
{
	RefArray< const world::EntityData > children;
	scene::Traverser::visit(entityData, [&](const world::EntityData* childEntityData) -> scene::Traverser::VisitorResult
	{
		children.push_back(childEntityData);
		return scene::Traverser::VrSkip;
	});

	os << entityData->getName() << L" " << entityData->getId().format() << Endl;

	if (!entityData->getComponents().empty())
	{
		os << L"Components:" << Endl;
		os << IncreaseIndent;
		for (auto component : entityData->getComponents())
			os << type_name(component) << Endl;
		os << DecreaseIndent;
	}

	if (!children.empty())
	{
		os << L"Children:" << Endl;
		os << IncreaseIndent;
		for (auto child : children)
			describeEntity(os, child);
		os << DecreaseIndent;
	}
}

/*! Add light to tracer scene.
 * \return True if light should be removed from scene (fully baked).
 */
bool addLight(const world::LightComponentData* lightComponentData, const Transform& transform, TracerTask* tracerTask)
{
	uint8_t mask = 0;

	auto bakeMode = lightComponentData->getBakeMode();
	if (bakeMode == world::LightComponentData::LbmIndirect)
		mask = Light::LmIndirect;
	else if (bakeMode == world::LightComponentData::LbmAll)
		mask = Light::LmDirect | Light::LmIndirect;

	if (!mask)
		return false;

	Light light;
	if (lightComponentData->getLightType() == world::LightType::Directional)
	{
		light.type = Light::LtDirectional;
		light.position = Vector4::origo();
		light.direction = -transform.axisY();
		light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
		light.range = Scalar(0.0f);
		light.mask = mask;
		tracerTask->addTracerLight(new TracerLight(light));
	}
	else if (lightComponentData->getLightType() == world::LightType::Point)
	{
		light.type = Light::LtPoint;
		light.position = transform.translation().xyz1();
		light.direction = Vector4::zero();
		light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
		light.range = Scalar(lightComponentData->getRange());
		light.mask = mask;
		tracerTask->addTracerLight(new TracerLight(light));
	}
	else if (lightComponentData->getLightType() == world::LightType::Spot)
	{
		light.type = Light::LtSpot;
		light.position = transform.translation().xyz1();
		light.direction = -transform.axisY();
		light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
		light.range = Scalar(lightComponentData->getRange());
		light.radius = Scalar(lightComponentData->getRadius());
		light.mask = mask;
		tracerTask->addTracerLight(new TracerLight(light));
	}

	return (bool)(bakeMode == world::LightComponentData::LbmAll);
}

/*! */
void addSky(
	editor::IPipelineBuilder* pipelineBuilder,
	const std::wstring& assetPath,
	const weather::SkyComponentData* skyComponentData,
	const Transform& skyTransform,
	float attenuation,
	TracerTask* tracerTask
)
{
	const auto& textureId = skyComponentData->getTexture();
	if (textureId.isNull())
		return;

	Ref< const render::TextureAsset > textureAsset = pipelineBuilder->getObjectReadOnly< render::TextureAsset >(textureId);
	if (!textureAsset)
		return;

	const Quaternion headRotation = Quaternion::fromEulerAngles(
		skyTransform.rotation().toEulerAngles().x(),
		0.0f,
		0.0f
	);

	const uint32_t textureAssetHash = pipelineBuilder->calculateInclusiveHash(textureAsset);

	Ref< IblProbe > probe = pipelineBuilder->getDataAccessCache()->read< IblProbe >(
		Key(0x00000001, 0x00000000, 0x00000000, textureAssetHash),
		[&](IStream* stream) -> Ref< IblProbe > {
			Ref< drawing::Image > radiance = drawing::Image::load(stream, L"tri");
			if (radiance)
				return new IblProbe(radiance);
			else
				return nullptr;
		},
		[=](const IblProbe* probe, IStream* stream) -> bool {
			if (!probe->getRadianceImage()->save(stream, L"tri"))
				return false;
			return true;
		},
		[&]() -> Ref< IblProbe > {
			// Read sky image from texture asset.
			Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + textureAsset->getFileName());
			Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
			if (!file)
				return nullptr;

			Ref< drawing::Image > skyImage = drawing::Image::load(file, textureAsset->getFileName().getExtension());
			if (!skyImage)
				return nullptr;

			safeClose(file);

			// Ensure image is of reasonable size, only used for low frequency data so size doesn't matter much.
			const int32_t dim = min(skyImage->getWidth(), skyImage->getHeight());
			if (dim > 128)
			{
				drawing::ScaleFilter scaleFilter(
					(skyImage->getWidth() * 128) / dim,
					(skyImage->getHeight() * 128) / dim,
					drawing::ScaleFilter::MnAverage,
					drawing::ScaleFilter::MgLinear
				);
				skyImage->apply(&scaleFilter);
			}

			Ref< const drawing::CubeMap > sourceRadianceCube = drawing::CubeMap::createFromImage(skyImage);
			T_FATAL_ASSERT(sourceRadianceCube);

			Ref< drawing::CubeMap > radianceCube = new drawing::CubeMap(128, drawing::PixelFormat::getRGBAF32());

			AlignedVector< Job::task_t > jobs;
			for (int32_t side = 0; side < 6; ++side)
			{
				jobs.push_back([=, &sourceRadianceCube, &radianceCube]() {
					Random random;
					for (int32_t y = 0; y < 128; ++y)
					{
						for (int32_t x = 0; x < 128; ++x)
						{
							const Vector4 d = headRotation * radianceCube->getDirection(side, x, y);
							Color4f cl(0.0f, 0.0f, 0.0f, 0.0f);
							Scalar totalWeight = 0.0_simd;
							for (int32_t i = 0; i < 10000; ++i)
							{
								const Vector2 uv = Quasirandom::hammersley(i, 10000, random);
								Vector4 direction = Quasirandom::uniformHemiSphere(uv, d);
								direction = lerp(d, direction, 0.125_simd).normalized();
								const Scalar weight = 1.0_simd; // dot3(d, direction);
								cl += sourceRadianceCube->get(direction) * weight;
								totalWeight += weight;
							}
							cl /= totalWeight;
							cl = max(cl, Color4f(0.0f, 0.0f, 0.0f, 0.0f));
							radianceCube->set(d, cl);
						}
					}
				});
			}
			JobManager::getInstance().fork(jobs.c_ptr(), jobs.size());

			// Convert cube map to equirectangular image.
			Ref< drawing::Image > radiance = radianceCube->createEquirectangular();
			T_FATAL_ASSERT(radiance != nullptr);

			// Discard alpha channel as they are not used.
			radiance->clearAlpha(1.0f);

			// Attenuate images; imperically measured using Blender as reference.
			const float c_refScale = 5.0f / 4.0f;
			const drawing::TransformFilter tform(Color4f(c_refScale, c_refScale, c_refScale, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));
			radiance->apply(&tform);

			return new IblProbe(radiance);
		}
	);

	// Scale probe by user attenuation factor.
	const drawing::TransformFilter tform(Color4f(attenuation, attenuation, attenuation, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));
	probe->apply(&tform);

	// Create tracer environment.
	tracerTask->addTracerEnvironment(new TracerEnvironment(probe));
}

/*! */
int32_t calculateLightmapSize(const model::Model* model, float lumelDensity, int32_t minimumSize, int32_t maximumSize)
{
	Winding3 polygonWinding;

	float totalWorldArea = 0.0f;
	for (const auto& polygon : model->getPolygons())
	{
		polygonWinding.resize(0);
		for (const auto index : polygon.getVertices())
			polygonWinding.push(model->getVertexPosition(index));
		totalWorldArea += abs(polygonWinding.area());
	}

	const float totalLightMapArea = lumelDensity * lumelDensity * totalWorldArea;
	int32_t size = (int32_t)(std::sqrt(totalLightMapArea) + 0.5f);
	size = std::max< int32_t >(minimumSize, size);
	size = std::min< int32_t >(maximumSize, size);

	return alignUp(size, 4);
}

/*! */
bool prepareModel(
	editor::IPipelineBuilder* pipelineBuilder,
	model::Model* model,
	const std::wstring& assetPath,
	int32_t lightmapSize
)
{
	// Ensure model is fit for tracing.
	model->clear(model::Model::CfColors | model::Model::CfJoints);
	model::Triangulate().apply(*model);
	model::CalculateTangents(false).apply(*model);

	// Check if model already contain lightmap UV or if we need to unwrap.
	uint32_t channel = model->getTexCoordChannel(L"Lightmap");
	if (channel == model::c_InvalidIndex)
	{
		// No lightmap UV channel, need to add and unwrap automatically.
		channel = model->addUniqueTexCoordChannel(L"Lightmap");
		model::UnwrapUV(channel, lightmapSize).apply(*model);
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.BakePipelineOperator", 0, BakePipelineOperator, scene::IScenePipelineOperator)

Ref< TracerProcessor > BakePipelineOperator::ms_tracerProcessor = nullptr;

BakePipelineOperator::BakePipelineOperator()
:	m_tracerType(nullptr)
,	m_asynchronous(false)
{
}

bool BakePipelineOperator::create(const editor::IPipelineSettings* settings)
{
	// Read all settings first so pipeline hash is consistent.
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	m_modelCachePath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.ModelCache.Path", L"");
	m_compressionMethod = settings->getPropertyIncludeHash< std::wstring >(L"TexturePipeline.CompressionMethod", L"DXTn");
	m_asynchronous = settings->getPropertyIncludeHash< bool >(L"Pipeline.TargetEditor", false) && !settings->getPropertyExcludeHash< bool >(L"Pipeline.TargetEditor.Build", false);

	// Instantiate raytracer implementation.
	const std::wstring tracerTypeName = settings->getPropertyIncludeHash< std::wstring >(L"BakePipelineOperator.RayTracerType", L"traktor.shape.RayTracerEmbree");
	m_tracerType = TypeInfo::find(tracerTypeName.c_str());
	if (!m_tracerType)
	{
		log::error << L"Unable to find tracer implementation \"" << tracerTypeName << L"\"." << Endl;
		return false;
	}

	// Do this last since we want to ensure asynchronous flag is properly set.
	const bool tracerEnable = settings->getPropertyIncludeHash< bool >(L"BakePipelineOperator.Enable", true);
	if (!tracerEnable)
		return true;

	// Create entity replicators.
	for (const auto entityReplicatorType : type_of< world::IEntityReplicator >().findAllOf(false))
	{
		Ref< world::IEntityReplicator > entityReplicator = mandatory_non_null_type_cast< world::IEntityReplicator* >(entityReplicatorType->createInstance());
		if (!entityReplicator->create(settings))
			return false;	

		auto supportedTypes = entityReplicator->getSupportedTypes();
		for (auto supportedType : supportedTypes)
			m_entityReplicators[supportedType] = entityReplicator;
	}

	// In case we're running synchronous mode we create our own tracer processor.
	if (!m_asynchronous)
		ms_tracerProcessor = new TracerProcessor(m_tracerType, m_compressionMethod, false);

	return true;
}

void BakePipelineOperator::destroy()
{
	if (!m_asynchronous && ms_tracerProcessor)
	{
		ms_tracerProcessor->waitUntilIdle();
		ms_tracerProcessor = nullptr;
	}
}

TypeInfoSet BakePipelineOperator::getOperatorTypes() const
{
	return makeTypeInfoSet< BakeConfiguration >();
}

void BakePipelineOperator::addDependencies(editor::IPipelineDepends* pipelineDepends) const
{
	pipelineDepends->addDependency(c_lightmapProxyId, editor::PdfBuild);
}

bool BakePipelineOperator::transform(
	editor::IPipelineCommon* pipelineCommon,
	const ISerializable* operatorData,
	scene::SceneAsset* inoutSceneAsset
) const
{
	const auto configuration = mandatory_non_null_type_cast< const BakeConfiguration* >(operatorData);

	// Skip transforming all to gether if no tracer type specified.
	if (!m_tracerType || !ms_tracerProcessor)
		return true;

	// Do not transform anything if we're not tracing lightmaps.
	if (!configuration->getEnableLightmaps())
		return true;

	RefArray< world::LayerEntityData > layers;
	for (const auto layer : inoutSceneAsset->getLayers())
	{
		// Resolve all external entities, inital seed is null since we don't want to modify entity ID on those
		// entities which are inlines in scene, only those referenced from an external entity should be re-assigned IDs.
		Ref< world::LayerEntityData > flattenedLayer = checked_type_cast< world::LayerEntityData* >(world::resolveExternal(
			[&](const Guid& objectId) -> Ref< const ISerializable > {
				return pipelineCommon->getObjectReadOnly(objectId);
			},
			layer,
			Guid::null,
			nullptr
		));
		if (!flattenedLayer)
			return false;

		// Dynamic layers do not get baked.
		if (auto editorAttributes = flattenedLayer->getComponent< world::EditorAttributesComponentData >())
		{
			if (!editorAttributes->include || editorAttributes->dynamic)
			{
				layers.push_back(flattenedLayer);
				continue;
			}
		}

		// Collect all entities from layer which do not get baked.
		scene::Traverser::visit(flattenedLayer, [&](Ref< world::EntityData >& inoutEntityData) -> scene::Traverser::VisitorResult
		{
			// Check editor attributes component if we should include entity.
			if (auto editorAttributes = inoutEntityData->getComponent< world::EditorAttributesComponentData >())
			{
				if (!editorAttributes->include)
					return scene::Traverser::VrSkip;

				if (editorAttributes->dynamic)
					return scene::Traverser::VrContinue;
			}

			// "Unnamed" entities do not bake.
			if (inoutEntityData->getId().isNull())
				return scene::Traverser::VrContinue;

			// Transform and keep entities which isn't included in bake.
			auto componentDatas = inoutEntityData->getComponents();
			for (auto componentData : componentDatas)
			{
				const world::IEntityReplicator* entityReplicator = m_entityReplicators[&type_of(componentData)];
				if (!entityReplicator)
					continue;

				Ref< world::GroupComponentData > outputGroup = new world::GroupComponentData();
				entityReplicator->transform(inoutEntityData, componentData, outputGroup);

				// Transfer entities from existing group.
				Ref< world::GroupComponentData > existingGroup = inoutEntityData->getComponent< world::GroupComponentData >();
				if (existingGroup)
				{
					Ref< world::EntityData > mergeEntity = new world::EntityData();
					mergeEntity->setId(Guid::create());
					mergeEntity->setComponent(existingGroup);
					outputGroup->addEntityData(mergeEntity);
				}

				inoutEntityData->setComponent(outputGroup);
				return scene::Traverser::VrSkip;
			}

			return scene::Traverser::VrContinue;
		});

		// Replace with modified layer in output scene.
		layers.push_back(flattenedLayer);
	}

	inoutSceneAsset->setLayers(layers);
	return true;
}

bool BakePipelineOperator::build(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* operatorData,
	const db::Instance* sourceInstance,
	scene::SceneAsset* inoutSceneAsset,
	bool rebuild
) const
{
	const auto configuration = mandatory_non_null_type_cast< const BakeConfiguration* >(operatorData);

	// Skip baking all to gather if no tracer type specified.
	if (!m_tracerType || !ms_tracerProcessor)
		return true;

	// Cancel any bake process currently running for given scene.
	ms_tracerProcessor->cancel(sourceInstance->getGuid());

	// Calculate hash of configuration so we don't use stale models from cache.
	const uint32_t configurationHash = DeepHash(configuration).get();

	log::info << L"Creating lightmap tasks..." << Endl;
	Ref< TracerTask > tracerTask = new TracerTask(
		sourceInstance->getGuid(),
		configuration
	);

	Aabb3 irradianceBoundingBox(
		Vector4(-100.0f, -100.0f, -100.0f),
		Vector4( 100.0f,  100.0f,  100.0f)
	);

	{
		RefArray< world::LayerEntityData > layers;
		SmallMap< Path, Ref< drawing::Image > > images;

		// Find all static meshes and lights; replace external referenced entities with local if necessary.
		for (const auto layer : inoutSceneAsset->getLayers())
		{
			// Resolve all external entities, initial seed is null since we don't want to modify entity ID on those
			// entities which are inline in scene, only those referenced from an external entity should be re-assigned IDs.
			Ref< world::LayerEntityData > flattenedLayer = checked_type_cast< world::LayerEntityData* >(world::resolveExternal(
				[&](const Guid& objectId) -> Ref< const ISerializable > {
					return pipelineBuilder->getObjectReadOnly(objectId);
				},
				layer,
				Guid::null,
				nullptr
			));
			if (!flattenedLayer)
				return false;

			// Dynamic layers do not get baked.
			if (auto editorAttributes = flattenedLayer->getComponent< world::EditorAttributesComponentData >())
			{
				if (!editorAttributes->include || editorAttributes->dynamic)
				{
					layers.push_back(flattenedLayer);
					continue;
				}
			}

			// Collect all entities from layer which we will include in bake.
			RefArray< world::EntityData > bakeEntityData;
			scene::Traverser::visit(flattenedLayer, [&](Ref< world::EntityData >& inoutEntityData) -> scene::Traverser::VisitorResult
				{
					// Check editor attributes component if we should include entity.
					if (auto editorAttributes = inoutEntityData->getComponent< world::EditorAttributesComponentData >())
					{
						if (!editorAttributes->include || editorAttributes->dynamic)
							return scene::Traverser::VrSkip;
					}

					// We only bake "named" entities.
					if (inoutEntityData->getId().isNull())
						return scene::Traverser::VrContinue;

					// Light, sky and irradiance volume must be included.
					if (
						inoutEntityData->getComponent< world::LightComponentData >() != nullptr ||
						inoutEntityData->getComponent< weather::SkyComponentData >() != nullptr ||
						inoutEntityData->getName() == L"Irradiance"
					)
					{
						bakeEntityData.push_back(inoutEntityData);
						return scene::Traverser::VrContinue;
					}

					// Include in bake.
					auto componentDatas = inoutEntityData->getComponents();
					for (auto componentData : componentDatas)
					{
						if (m_entityReplicators.find(&type_of(componentData)) != m_entityReplicators.end())
						{
							bakeEntityData.push_back(inoutEntityData);
							return scene::Traverser::VrSkip;
						}
					}

					return scene::Traverser::VrContinue;
				});

			// Traverse and visit all entities in layer.
			for (auto inoutEntityData : bakeEntityData)
			{
				// Add light source.
				if (auto lightComponentData = inoutEntityData->getComponent< world::LightComponentData >())
				{
					if (addLight(lightComponentData, inoutEntityData->getTransform(), tracerTask))
						inoutEntityData->removeComponent(lightComponentData);
				}

				// Add sky source.
				if (auto skyComponentData = inoutEntityData->getComponent< weather::SkyComponentData >())
					addSky(pipelineBuilder, m_assetPath, skyComponentData, inoutEntityData->getTransform(), configuration->getSkyAttenuation(), tracerTask);

				// Get volume for irradiance grid.
				if (inoutEntityData->getName() == L"Irradiance")
				{
					if (auto volumeComponentData = inoutEntityData->getComponent< world::VolumeComponentData >())
						irradianceBoundingBox = volumeComponentData->getVolumes().front().transform(inoutEntityData->getTransform());
				}

				// Calculate synthesized ids.
				const Guid entityId = inoutEntityData->getId();
				Guid lightmapDiffuseId = entityId.permutation(c_lightmapDiffuseIdSeed);
				Guid lightmapDirectionalId = entityId.permutation(c_lightmapDirectionalIdSeed);
				Guid outputId = entityId.permutation(c_outputIdSeed);

				// Find model synthesizer which can generate from components.
				auto componentDatas = inoutEntityData->getComponents();
				for (auto componentData : componentDatas)
				{
					const world::IEntityReplicator* entityReplicator = m_entityReplicators[&type_of(componentData)];
					if (!entityReplicator)
						continue;

					const uint32_t componentDataHash = pipelineBuilder->calculateInclusiveHash(componentData);
					const uint32_t modelHash = configurationHash + componentDataHash;

					Ref< model::Model > visualModel = pipelineBuilder->getDataAccessCache()->read< model::Model >(
						Key(0x00000020, 0x00000000, type_of(entityReplicator).getVersion(), modelHash),
						[&](IStream* stream) -> Ref< model::Model > {
							return BinarySerializer(stream).readObject< model::Model >();
						},
						[=](const model::Model* model, IStream* stream) -> bool {
							return BinarySerializer(stream).writeObject(model);
						},
							[&]() -> Ref< model::Model > {
							pipelineBuilder->getProfiler()->begin(type_of(entityReplicator));
							Ref< model::Model > model = entityReplicator->createModel(pipelineBuilder, inoutEntityData, componentData, world::IEntityReplicator::Usage::Visual);
							pipelineBuilder->getProfiler()->end(type_of(entityReplicator));
							if (!model)
								return nullptr;

							// Calculate size of lightmap from geometry.
							const int32_t lightmapSize = calculateLightmapSize(
								model,
								configuration->getLumelDensity(),
								configuration->getMinimumLightMapSize(),
								configuration->getMaximumLightMapSize()
							);

							// Prepare model for baking.
							if (!prepareModel(
								pipelineBuilder,
								model,
								m_assetPath,
								lightmapSize
							))
								return nullptr;

							model->setProperty< PropertyInteger >(L"LightmapSize", lightmapSize);
							return model;
						}
					);

					Ref< model::Model > collisionModel = pipelineBuilder->getDataAccessCache()->read< model::Model >(
						Key(0x00000030, 0x00000000, type_of(entityReplicator).getVersion(), modelHash),
						[&](IStream* stream) -> Ref< model::Model > {
							return BinarySerializer(stream).readObject< model::Model >();
						},
						[=](const model::Model* model, IStream* stream) -> bool {
							return BinarySerializer(stream).writeObject(model);
						},
							[&]() -> Ref< model::Model > {
							pipelineBuilder->getProfiler()->begin(type_of(entityReplicator));
							Ref< model::Model > model = entityReplicator->createModel(pipelineBuilder, inoutEntityData, componentData, world::IEntityReplicator::Usage::Collision);
							pipelineBuilder->getProfiler()->end(type_of(entityReplicator));
							return model;
						}
					);

					// Add visual model to tracer task.
					if (visualModel)
					{
						log::info << L"Adding model \"" << inoutEntityData->getName() << L"\" (" << type_name(entityReplicator) << L") to tracer task..." << Endl;

						const int32_t lightmapSize = visualModel->getProperty< int32_t >(L"LightmapSize");
						bool needDirectionalMap = false;

						// Modify all materials to contain reference to lightmap channel.
						for (auto& material : visualModel->getMaterials())
						{
							material.setLightMap(model::Material::Map(L"Lightmap", L"Lightmap", false, lightmapDiffuseId));
							if (configuration->getEnableDirectionalMaps() && !material.getNormalMap().name.empty())
							{
								material.setProperty< PropertyString >(L"LightMapDirectionalId", lightmapDirectionalId.format());
								needDirectionalMap = true;
							}
						}

						// Load texture images and attach to materials.
						for (auto& material : visualModel->getMaterials())
						{
							auto diffuseMap = material.getDiffuseMap();
							if (diffuseMap.texture.isNotNull())
							{
								Ref< const render::TextureAsset > textureAsset = pipelineBuilder->getObjectReadOnly< render::TextureAsset >(diffuseMap.texture);
								if (!textureAsset)
									continue;

								Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + textureAsset->getFileName());
								Ref< drawing::Image > image = images[filePath];
								if (image == nullptr)
								{
									Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
									if (file)
									{
										image = drawing::Image::load(file, textureAsset->getFileName().getExtension());
										if (image && !textureAsset->m_output.m_linearGamma)
										{
											// Convert to linear color space.
											const drawing::GammaFilter gammaFilter(1.0f / 2.2f);
											image->apply(&gammaFilter);
										}
										images[filePath] = image;
									}
								}

								diffuseMap.image = image;
								material.setDiffuseMap(diffuseMap);
							}
						}

						tracerTask->addTracerModel(new TracerModel(
							visualModel,
							inoutEntityData->getTransform()
						));

						if (configuration->getEnableLightmaps())
						{
							Ref< db::Instance > lightmapDiffuseInstance;
							if (lightmapDiffuseId.isNotNull())
							{
								lightmapDiffuseInstance = pipelineBuilder->createOutputInstance(L"Generated/" + lightmapDiffuseId.format(), lightmapDiffuseId);
								if (!lightmapDiffuseInstance)
									return false;
								lightmapDiffuseInstance->setObject(new render::AliasTextureResource(
									resource::Id< render::ITexture >(c_lightmapProxyId)
								));
								lightmapDiffuseInstance->commit();
							}

							Ref< db::Instance > lightmapDirectionalInstance;
							if (needDirectionalMap && lightmapDirectionalId.isNotNull())
							{
								lightmapDirectionalInstance = pipelineBuilder->createOutputInstance(L"Generated/" + lightmapDirectionalId.format(), lightmapDirectionalId);
								if (!lightmapDirectionalInstance)
									return false;
								lightmapDirectionalInstance->setObject(new render::AliasTextureResource(
									resource::Id< render::ITexture >(c_lightmapProxyId)
								));
								lightmapDirectionalInstance->commit();
							}

							tracerTask->addTracerOutput(new TracerOutput(
								lightmapDiffuseInstance,
								lightmapDirectionalInstance,
								visualModel,
								inoutEntityData->getTransform(),
								lightmapSize
							));
						}
					}

					// Modify entity.
					if (configuration->getEnableLightmaps() && (visualModel || collisionModel))
					{
						Ref< world::GroupComponentData > outputGroup = new world::GroupComponentData();
						entityReplicator->transform(inoutEntityData, componentData, outputGroup);

						if (visualModel)
						{
							//Ref< const mesh::MeshAsset > meshAsset = dynamic_type_cast< const mesh::MeshAsset* >(visualModel->getProperty< ISerializable >(world::IEntityReplicator::VisualMesh));

							// Create and build a new mesh asset referencing the modified model.
							Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
							outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
							//if (meshAsset)
							//{
							//	outputMeshAsset->setMaterialTemplates(meshAsset->getMaterialTemplates());
							//	outputMeshAsset->setMaterialTextures(meshAsset->getMaterialTextures());
							//}

							Ref< world::EntityData > outputMeshEntity = new world::EntityData();
							outputMeshEntity->setId(Guid::create());
							outputMeshEntity->setName(inoutEntityData->getName());
							outputMeshEntity->setTransform(inoutEntityData->getTransform());
							outputMeshEntity->setComponent(new mesh::MeshComponentData(resource::Id< mesh::IMesh >(outputId)));
							outputGroup->addEntityData(outputMeshEntity);

							// Ensure visual mesh is build.
							pipelineBuilder->buildAdHocOutput(
								outputMeshAsset,
								outputId,
								visualModel
							);
						}

						if (collisionModel)
						{
							const Guid outputShapeId = outputId.permutation(c_shapeMeshAssetSeed);

							//Ref< const physics::MeshAsset > meshAsset = dynamic_type_cast< const physics::MeshAsset* >(collisionModel->getProperty< ISerializable >(world::IEntityReplicator::CollisionMesh));
							//Ref< const physics::ShapeDesc > shapeDesc = dynamic_type_cast< const physics::ShapeDesc* >(collisionModel->getProperty< ISerializable >(world::IEntityReplicator::CollisionShape));
							//Ref< const physics::StaticBodyDesc > bodyDesc = dynamic_type_cast< const physics::StaticBodyDesc* >(collisionModel->getProperty< ISerializable >(world::IEntityReplicator::CollisionBody));

							// Build collision shape mesh.
							Ref< physics::MeshAsset > outputMeshAsset = new physics::MeshAsset();
							outputMeshAsset->setCalculateConvexHull(false);
							//if (meshAsset)
							//	outputMeshAsset->setMaterials(meshAsset->getMaterials());

							Ref< physics::MeshShapeDesc > outputShapeDesc = new physics::MeshShapeDesc(resource::Id< physics::Mesh >(outputShapeId));
							//if (shapeDesc)
							//{
							//	outputShapeDesc->setCollisionGroup(shapeDesc->getCollisionGroup());
							//	outputShapeDesc->setCollisionMask(shapeDesc->getCollisionMask());
							//}

							Ref< physics::StaticBodyDesc > outputBodyDesc = new physics::StaticBodyDesc(outputShapeDesc);
							//if (bodyDesc)
							//{
							//	outputBodyDesc->setFriction(bodyDesc->getFriction());
							//	outputBodyDesc->setRestitution(bodyDesc->getRestitution());
							//}

							Ref< world::EntityData > outputShapeEntity = new world::EntityData();
							outputShapeEntity->setId(Guid::create());
							outputShapeEntity->setComponent(new physics::RigidBodyComponentData(outputBodyDesc));
							outputGroup->addEntityData(outputShapeEntity);

							// Ensure collision shape is built.
							pipelineBuilder->buildAdHocOutput(
								outputMeshAsset,
								outputShapeId,
								collisionModel
							);
						}

						// Transfer entities from existing group, need to merge with our output group.
						Ref< world::GroupComponentData > existingGroup = inoutEntityData->getComponent< world::GroupComponentData >();
						if (existingGroup)
						{
							for (auto childEntity : existingGroup->getEntityData())
								outputGroup->addEntityData(childEntity);
						}

						inoutEntityData->setComponent(outputGroup);
					}

					lightmapDiffuseId.permutate();
					lightmapDirectionalId.permutate();
					outputId.permutate();
				}
			}

			// Replace with modified layer in output scene.
			layers.push_back(flattenedLayer);
		}
		inoutSceneAsset->setLayers(layers);
		images.clear();
	}

	// Create irradiance grid task.
	{
		const Guid c_irradianceGridIdSeed(L"{714D9AF6-EF62-4E15-B372-7CEBB090417B}");
		Guid irradianceGridId = sourceInstance->getGuid().permutation(c_irradianceGridIdSeed);

		// Create irradiance instance.
		Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
			L"Generated/" + irradianceGridId.format(),
			irradianceGridId
		);
		if (!outputInstance)
		{
			log::error << L"BakePipelineOperator failed; unable to create output instance." << Endl;
			return false;
		}

		Ref< world::IrradianceGridResource > outputResource = new world::IrradianceGridResource();
		outputInstance->setObject(outputResource);

		// Create output data stream.
		Ref< IStream > stream = outputInstance->writeData(L"Data");
		if (!stream)
		{
			log::error << L"BakePipelineOperator failed; unable to create irradiance data stream." << Endl;
			outputInstance->revert();
			return false;
		}

		Writer writer(stream);
		writer << uint32_t(2);
		writer << (uint32_t)1;	// width
		writer << (uint32_t)1;	// height
		writer << (uint32_t)1;	// depth
		writer << -10000.0f;
		writer << -10000.0f;
		writer << -10000.0f;
		writer <<  10000.0f;
		writer <<  10000.0f;
		writer <<  10000.0f;

		for (int32_t i = 0; i < 9; ++i)
		{
			writer << 0.0f;
			writer << 0.0f;
			writer << 0.0f;
		}

		stream->close();

		if (!outputInstance->commit())
		{
			log::error << L"BakePipelineOperator failed; unable to commit output instance." << Endl;
			return false;
		}

		tracerTask->addTracerIrradiance(new TracerIrradiance(
			outputInstance,
			irradianceBoundingBox
		));

		// Modify scene with our generated irradiance grid resource.
		inoutSceneAsset->getWorldRenderSettings()->irradianceGrid = resource::Id< world::IrradianceGrid >(
			irradianceGridId
		);
	}

	// Finally enqueue task to tracer processor.
	ms_tracerProcessor->enqueue(tracerTask);

	if (m_asynchronous)
		log::info << L"Lightmap tasks created, enqueued and ready to be processed." << Endl;
	else
	{
		log::info << L"Waiting for lightmap baking to complete..." << Endl;
		ms_tracerProcessor->waitUntilIdle();
	}

	return true;
}

void BakePipelineOperator::setTracerProcessor(TracerProcessor* tracerProcessor)
{
	ms_tracerProcessor = tracerProcessor;
}

TracerProcessor* BakePipelineOperator::getTracerProcessor()
{
	return ms_tracerProcessor;
}

	}
}
