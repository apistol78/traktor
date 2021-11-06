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
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/UnwrapUV.h"
#include "Render/Types.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Resource/AliasTextureResource.h"
#include "Scene/Editor/IEntityReplicator.h"
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
	if (lightComponentData->getLightType() == world::LightType::LtDirectional)
	{
		light.type = Light::LtDirectional;
		light.position = Vector4::origo();
		light.direction = -transform.axisY();
		light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
		light.range = Scalar(0.0f);
		light.mask = mask;
		tracerTask->addTracerLight(new TracerLight(light));
	}
	else if (lightComponentData->getLightType() == world::LightType::LtPoint)
	{
		light.type = Light::LtPoint;
		light.position = transform.translation().xyz1();
		light.direction = Vector4::zero();
		light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
		light.range = Scalar(lightComponentData->getRange());
		light.mask = mask;
		tracerTask->addTracerLight(new TracerLight(light));
	}
	else if (lightComponentData->getLightType() == world::LightType::LtSpot)
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

	uint32_t textureAssetHash = pipelineBuilder->calculateInclusiveHash(textureAsset);

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
			int32_t dim = min(skyImage->getWidth(), skyImage->getHeight());
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

			RefArray< Job > jobs;
			for (int32_t side = 0; side < 6; ++side)
			{
				Ref< Job > job = JobManager::getInstance().add([=, &sourceRadianceCube, &radianceCube]() {
					Random random;
					for (int32_t y = 0; y < 128; ++y)
					{
						for (int32_t x = 0; x < 128; ++x)
						{
							Vector4 d = headRotation * radianceCube->getDirection(side, x, y);
							Color4f cl(0.0f, 0.0f, 0.0f, 0.0f);
							Scalar totalWeight = 0.0_simd;
							for (int32_t i = 0; i < 10000; ++i)
							{
								Vector2 uv = Quasirandom::hammersley(i, 10000, random);
								Vector4 direction = Quasirandom::uniformHemiSphere(uv, d);
								direction = lerp(d, direction, 0.125_simd).normalized();
								Scalar weight = 1.0_simd; // dot3(d, direction);
								cl += sourceRadianceCube->get(direction) * weight;
								totalWeight += weight;
							}
							cl /= totalWeight;
							cl = max(cl, Color4f(0.0f, 0.0f, 0.0f, 0.0f));
							radianceCube->set(d, cl);
						}
					}
				});
				jobs.push_back(job);
			}
			while (!jobs.empty())
			{
				jobs.back()->wait();
				jobs.pop_back();
			}

			// Convert cube map to equirectangular image.
			Ref< drawing::Image > radiance = radianceCube->createEquirectangular();
			T_FATAL_ASSERT(radiance != nullptr);

			// Discard alpha channel as they are not used.
			radiance->clearAlpha(1.0f);

			// Attenuate images; imperically measured using Blender as reference.
			const float c_refScale = 5.0f / 4.0f;
			drawing::TransformFilter tform(Color4f(c_refScale, c_refScale, c_refScale, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));
			radiance->apply(&tform);

			return new IblProbe(radiance);
		}
	);

	// Scale probe by user attenuation factor.
	drawing::TransformFilter tform(Color4f(attenuation, attenuation, attenuation, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));
	probe->apply(&tform);

	// Create tracer environment.
	tracerTask->addTracerEnvironment(new TracerEnvironment(probe));
}

/*! */
int32_t calculateLightmapSize(const model::Model* model, float lumelDensity, int32_t minimumSize, int32_t maximumSize)
{
	float totalWorldArea = 0.0f;
	for (const auto& polygon : model->getPolygons())
	{
		Winding3 polygonWinding;
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
	std::wstring tracerTypeName = settings->getPropertyIncludeHash< std::wstring >(L"BakePipelineOperator.RayTracerType", L"traktor.shape.RayTracerEmbree");
	m_tracerType = TypeInfo::find(tracerTypeName.c_str());
	if (!m_tracerType)
	{
		log::error << L"Unable to find tracer implementation \"" << tracerTypeName << L"\"." << Endl;
		return false;
	}

	// Do this last since we want to ensure asynchronous flag is properly set.
	bool tracerEnable = settings->getPropertyIncludeHash< bool >(L"BakePipelineOperator.Enable", true);
	if (!tracerEnable)
		return true;

	// Create entity replicators.
	TypeInfoSet entityReplicatorTypes;
	type_of< scene::IEntityReplicator >().findAllOf(entityReplicatorTypes, false);
	for (const auto& entityReplicatorType : entityReplicatorTypes)
	{
		Ref< scene::IEntityReplicator > entityReplicator = mandatory_non_null_type_cast< scene::IEntityReplicator* >(entityReplicatorType->createInstance());
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

bool BakePipelineOperator::addDependencies(editor::IPipelineDepends* pipelineDepends, const ISerializable* operatorData, const scene::SceneAsset* sceneAsset) const
{
	AlignedVector< Guid > externalEntityIds;

	pipelineDepends->addDependency< render::ShaderGraph >();
	pipelineDepends->addDependency(c_lightmapProxyId, editor::PdfBuild);

	// Add "use" dependencies to first level of external entity datas; so we ensure scene pipeline is invoked if external entity is modified.
	for (const auto layer : sceneAsset->getLayers())
	{
		// Resolve all external entities, inital seed is null since we don't want to modify entity ID on those
		// entities which are inlines in scene, only those referenced from an external entity should be re-assigned IDs.
		Ref< world::LayerEntityData > flattenedLayer = checked_type_cast< world::LayerEntityData* >(world::resolveExternal(pipelineDepends, layer, Guid::null, &externalEntityIds));
		if (!flattenedLayer)
			return false;

		// Do not add dynamic layers to bake.
		if (auto editorAttributes = flattenedLayer->getComponent< world::EditorAttributesComponentData >())
		{
			if (!editorAttributes->include || editorAttributes->dynamic)
				continue;
		}

		scene::Traverser::visit(flattenedLayer, [&](const world::EntityData* entityData) -> scene::Traverser::VisitorResult
		{
			if (auto editorAttributes = entityData->getComponent< world::EditorAttributesComponentData >())
			{
				if (!editorAttributes->include || editorAttributes->dynamic)
					return scene::Traverser::VrSkip;
			}

			// Find model synthesizer which can add dependencies from components.
			for (auto componentData : entityData->getComponents())
			{
				const scene::IEntityReplicator* entityReplicator = m_entityReplicators[&type_of(componentData)];
				if (entityReplicator)
				{
					pipelineDepends->addDependency(type_of(entityReplicator));
					entityReplicator->addDependencies(pipelineDepends, entityData, componentData);
				}
			}

			return scene::Traverser::VrContinue;
		});
	}

	// Add dependency to all external entities.
	for (const auto& externalEntityId : externalEntityIds)
		pipelineDepends->addDependency(externalEntityId, editor::PdfUse);

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

	// Skip baking all to gether if no tracer type specified.
	if (!m_tracerType || !ms_tracerProcessor)
		return true;

	// Cancel any bake process currently running for given scene.
	ms_tracerProcessor->cancel(sourceInstance->getGuid());

	Timer timer;
	timer.reset();

	log::info << L"Creating lightmap tasks..." << Endl;

	Ref< TracerTask > tracerTask = new TracerTask(
		sourceInstance->getGuid(),
		configuration
	);

	RefArray< world::LayerEntityData > layers;
	SmallMap< Path, Ref< drawing::Image > > images;
	Aabb3 irradianceBoundingBox;

	// Find all static meshes and lights; replace external referenced entities with local if necessary.
	for (const auto layer : inoutSceneAsset->getLayers())
	{
		// Resolve all external entities, inital seed is null since we don't want to modify entity ID on those
		// entities which are inlines in scene, only those referenced from an external entity should be re-assigned IDs.
		Ref< world::LayerEntityData > flattenedLayer = checked_type_cast< world::LayerEntityData* >(world::resolveExternal(pipelineBuilder, layer, Guid::null, nullptr));
		if (!flattenedLayer)
			return false;

		// Do not add dynamic layers to bake.
		if (auto editorAttributes = flattenedLayer->getComponent< world::EditorAttributesComponentData >())
		{
			if (!editorAttributes->include || editorAttributes->dynamic)
			{
				layers.push_back(flattenedLayer);
				continue;
			}
		}

#if 0
		// Log pre-"layer entity hierarchy".
		{
			FileSystem::getInstance().makeAllDirectories(L"temp/Bake");
			Ref< IStream > f = FileSystem::getInstance().open(L"temp/Bake/" + sourceInstance->getName() + L" " + flattenedLayer->getName() + L" (Before).txt", File::FmWrite);
			if (f)
			{
				Ref< FileOutputStream > fos = new FileOutputStream(f, new Utf8Encoding());
				describeEntity(*fos, flattenedLayer);
				fos->close();
				f->close();
			}
		}
#endif

		// Collect all entities from layer.
		RefArray< world::EntityData > flattenEntityData;
		scene::Traverser::visit(flattenedLayer, [&](Ref< world::EntityData >& inoutEntityData) -> scene::Traverser::VisitorResult
		{
			// Check editor attributes component if we should include entity.
			if (auto editorAttributes = inoutEntityData->getComponent< world::EditorAttributesComponentData >())
			{
				if (!editorAttributes->include || editorAttributes->dynamic)
					return scene::Traverser::VrSkip;
			}

			// Only accept "named" entities.
			Guid entityId = inoutEntityData->getId();
			if (!entityId.isNull())
				flattenEntityData.push_back(inoutEntityData);

			// Stop traversing deeper if this entity has an replicator, ie will be baked.
			bool haveReplicator = false;
			for (auto componentData : inoutEntityData->getComponents())
			{
				const scene::IEntityReplicator* entityReplicator = m_entityReplicators[&type_of(componentData)];
				if (entityReplicator)
				{
					haveReplicator = true;
					break;
				}
			}
			if (haveReplicator)
				return scene::Traverser::VrSkip;

			return scene::Traverser::VrContinue;
		});

#if 0
		// Log flatten-"layer entity hierarchy".
		{
			FileSystem::getInstance().makeAllDirectories(L"temp/Bake");
			Ref< IStream > f = FileSystem::getInstance().open(L"temp/Bake/" + sourceInstance->getName() + L" " + flattenedLayer->getName() + L" (Flatten).txt", File::FmWrite);
			if (f)
			{
				Ref< FileOutputStream > fos = new FileOutputStream(f, new Utf8Encoding());
				for (auto flatten : flattenEntityData)
					describeEntity(*fos, flatten);
				fos->close();
				f->close();
			}
		}
#endif

		// Traverse and visit all entities in layer.
		for (auto inoutEntityData : flattenEntityData)
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
			Guid entityId = inoutEntityData->getId();
			Guid lightmapDiffuseId = entityId.permutation(c_lightmapDiffuseIdSeed);
			Guid lightmapDirectionalId = entityId.permutation(c_lightmapDirectionalIdSeed);
			Guid outputId = entityId.permutation(c_outputIdSeed);

			// Find model synthesizer which can generate from components.
			RefArray< world::IEntityComponentData > componentDatas = inoutEntityData->getComponents();
			for (auto componentData : componentDatas)
			{
				const scene::IEntityReplicator* entityReplicator = m_entityReplicators[&type_of(componentData)];
				if (!entityReplicator)
					continue;

				uint32_t componentDataHash = pipelineBuilder->calculateInclusiveHash(componentData);

				Ref< model::Model > model = pipelineBuilder->getDataAccessCache()->read< model::Model >(
					Key(0x00000002, 0x00000000, type_of(entityReplicator).getVersion(), componentDataHash),
					[&](IStream* stream) -> Ref< model::Model > {
						return BinarySerializer(stream).readObject< model::Model >();
					},
					[=](const model::Model* model, IStream* stream) -> bool {
						return BinarySerializer(stream).writeObject(model);
					},
					[&]() -> Ref< model::Model > {
						pipelineBuilder->getProfiler()->begin(type_of(entityReplicator));
						Ref< model::Model > model = entityReplicator->createModel(pipelineBuilder, m_assetPath, inoutEntityData, componentData);
						pipelineBuilder->getProfiler()->end(type_of(entityReplicator));
						if (!model)
							return nullptr;

						// Calculate size of lightmap from geometry.
						int32_t lightmapSize = calculateLightmapSize(
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

						return model;
					}
				);
				if (!model)
					return false;

				// Calculate size of lightmap from geometry.
				int32_t lightmapSize = calculateLightmapSize(
					model,
					configuration->getLumelDensity(),
					configuration->getMinimumLightMapSize(),
					configuration->getMaximumLightMapSize()
				);

				bool needDirectionalMap = false;

				// Modify all materials to contain reference to lightmap channel.
				for (auto& material : model->getMaterials())
				{
					material.setLightMap(model::Material::Map(L"Lightmap", L"Lightmap", false, lightmapDiffuseId));
					if (configuration->getEnableDirectionalMaps() && !material.getNormalMap().name.empty())
					{
						material.setProperty< PropertyString >(L"LightMapDirectionalId", lightmapDirectionalId.format());
						needDirectionalMap = true;
					}
				}

				// Load texture images and attach to materials.
				for (auto& material : model->getMaterials())
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
									drawing::GammaFilter gammaFilter(1.0f / 2.2f);
									image->apply(&gammaFilter);							
								}
								images[filePath] = image;
							}
						}

						diffuseMap.image = image;			
						material.setDiffuseMap(diffuseMap);
					}
				}

				Ref< db::Instance > lightmapDiffuseInstance;
				if (lightmapDiffuseId.isNotNull())
				{
					lightmapDiffuseInstance = pipelineBuilder->createOutputInstance(L"Generated/" + lightmapDiffuseId.format(), lightmapDiffuseId);
					if (!lightmapDiffuseInstance)
						return false;
					lightmapDiffuseInstance->setObject(new render::AliasTextureResource(
						resource::Id< render::ITexture >(c_lightmapProxyId)
					));
					lightmapDiffuseInstance->commit(db::CfKeepCheckedOut);
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
					lightmapDirectionalInstance->commit(db::CfKeepCheckedOut);
				}

				tracerTask->addTracerModel(new TracerModel(
					model,
					inoutEntityData->getTransform()
				));

				tracerTask->addTracerOutput(new TracerOutput(
					lightmapDiffuseInstance,
					lightmapDirectionalInstance,
					model,
					inoutEntityData->getTransform(),
					lightmapSize
				));

				// Let model generator consume altered model and modify entity in ways
				// which make sense for entity data.
				{
					pipelineBuilder->getProfiler()->begin(type_of(entityReplicator));
					Ref< world::IEntityComponentData > replaceComponentData = checked_type_cast< world::IEntityComponentData* >(entityReplicator->modifyOutput(
						pipelineBuilder,
						m_assetPath,
						inoutEntityData,
						componentData,
						model,
						outputId
					));
					pipelineBuilder->getProfiler()->end(type_of(entityReplicator));
					if (replaceComponentData == nullptr)
						inoutEntityData->removeComponent(componentData);
					else if (replaceComponentData != componentData)
					{
						inoutEntityData->removeComponent(componentData);
						inoutEntityData->setComponent(replaceComponentData);
					}
				}

				lightmapDiffuseId.permutate();
				lightmapDirectionalId.permutate();
				outputId.permutate();
			}
		}

#if 0
		// Log post-"layer entity hierarchy".
		{
			FileSystem::getInstance().makeAllDirectories(L"temp/Bake");
			Ref< IStream > f = FileSystem::getInstance().open(L"temp/Bake/" + sourceInstance->getName() + L" " + flattenedLayer->getName() + L" (After).txt", File::FmWrite);
			if (f)
			{
				Ref< FileOutputStream > fos = new FileOutputStream(f, new Utf8Encoding());
				describeEntity(*fos, flattenedLayer);
				fos->close();
				f->close();
			}
		}
#endif

		// Replace with modified layer in output scene.
		layers.push_back(flattenedLayer);
	}
	inoutSceneAsset->setLayers(layers);
	images.clear();

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

		// Commit a black irradiance grid first until async tracer has finished.
		if (m_asynchronous)
		{
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

			if (!outputInstance->commit(db::CfKeepCheckedOut))
			{
				log::error << L"BakePipelineOperator failed; unable to commit output instance." << Endl;
				return false;
			}
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
		log::info << L"Lightmap tasks created, enqueued and ready to be processed (" << formatDuration(timer.getElapsedTime()) << L")." << Endl;
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
