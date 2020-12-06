#include <limits>
#include <functional>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/Range.h"
#include "Core/Math/Winding3.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Filters/TransformFilter.h"
#include "Editor/DataAccessCache.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
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
#include "Render/Editor/Texture/CubeMap.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Resource/TextureResource.h"
#include "Scene/Editor/IEntityReplicator.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/Traverser.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/BakePipelineOperator.h"
#include "Shape/Editor/Bake/BakeReceipt.h"
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
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/LightComponentData.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

const Guid c_lightmapIdSeed(L"{A5A16214-0A01-4D6D-A509-6A5A16ACB6A3}");
const Guid c_outputIdSeed(L"{043B98C3-F93B-4510-8B73-1B5EEF2323E5}");

/*! Resolve external entities, ie flatten scene without external references. */
Ref< ISerializable > resolveAllExternal(editor::IPipelineCommon* pipeline, const ISerializable* object, const Guid& seed)
{
	Ref< Reflection > reflection = Reflection::create(object);

 	RefArray< ReflectionMember > objectMembers;
 	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	for (auto member : objectMembers)
	{
		RfmObject* objectMember = dynamic_type_cast< RfmObject* >(member);
		if (const world::ExternalEntityData* externalEntityDataRef = dynamic_type_cast< const world::ExternalEntityData* >(objectMember->get()))
		{
			Ref< const ISerializable > externalEntityData = pipeline->getObjectReadOnly(externalEntityDataRef->getEntityData());
			if (!externalEntityData)
				return nullptr;

			Guid entityDataId = externalEntityDataRef->getId().permutation(seed);

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveAllExternal(pipeline, externalEntityData, entityDataId));
			if (!resolvedEntityData)
				return nullptr;

			resolvedEntityData->setId(entityDataId);
			resolvedEntityData->setName(externalEntityDataRef->getName());
			resolvedEntityData->setTransform(externalEntityDataRef->getTransform());
			objectMember->set(resolvedEntityData);
		}
		else if (const world::EntityData* entityDataRef = dynamic_type_cast< const world::EntityData* >(objectMember->get()))
		{
			Guid entityDataId = entityDataRef->getId().permutation(seed);

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveAllExternal(pipeline, entityDataRef, entityDataId));
			if (!resolvedEntityData)
				return nullptr;

			resolvedEntityData->setId(entityDataId);
			objectMember->set(resolvedEntityData);
		}
		else if (objectMember->get())
		{
			objectMember->set(resolveAllExternal(pipeline, objectMember->get(), seed));
		}
	}

	return reflection->clone();
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
	if (lightComponentData->getLightType() == world::LtDirectional)
	{
		light.type = Light::LtDirectional;
		light.position = Vector4::origo();
		light.direction = -transform.axisY();
		light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
		light.range = Scalar(0.0f);
		light.mask = mask;
		tracerTask->addTracerLight(new TracerLight(light));
	}
	else if (lightComponentData->getLightType() == world::LtPoint)
	{
		light.type = Light::LtPoint;
		light.position = transform.translation().xyz1();
		light.direction = Vector4::zero();
		light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
		light.range = Scalar(lightComponentData->getRange());
		light.mask = mask;
		tracerTask->addTracerLight(new TracerLight(light));
	}
	else if (lightComponentData->getLightType() == world::LtSpot)
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
	TracerTask* tracerTask
)
{
	const auto& textureId = skyComponentData->getTexture();
	if (textureId.isNull())
		return;

	Ref< const render::TextureAsset > textureAsset = pipelineBuilder->getObjectReadOnly< render::TextureAsset >(textureId);
	if (!textureAsset)
		return;

	Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + textureAsset->getFileName());
	Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
	if (!file)
		return;

	Ref< drawing::Image > skyImage = drawing::Image::load(file, textureAsset->getFileName().getExtension());
	if (!skyImage)
		return;

	safeClose(file);

	// Ensure image is of reasonable size, only used for low frequency data so size doesn't matter much.
	int32_t dim = min(skyImage->getWidth(), skyImage->getHeight());
	if (dim > 256)
	{
		drawing::ScaleFilter scaleFilter(
			(skyImage->getWidth() * 256) / dim,
			(skyImage->getHeight() * 256) / dim,
			drawing::ScaleFilter::MnAverage,
			drawing::ScaleFilter::MgLinear
		);
		skyImage->apply(&scaleFilter);
	}

	Ref< const render::CubeMap > cms = render::CubeMap::createFromImage(skyImage);
	Ref< render::CubeMap > cmd = new render::CubeMap(128, drawing::PixelFormat::getRGBAF32());

	// Convolve cube map.
	Random random;
	for (int32_t side = 0; side < 6; ++side)
	{
		for (int32_t y = 0; y < 128; ++y)
		{
			for (int32_t x = 0; x < 128; ++x)
			{
				Vector4 d = cmd->getDirection(side, x, y);
				Color4f cl(0.0f, 0.0f, 0.0f, 0.0f);
				for (int32_t i = 0; i < 250; ++i)
				{
					Vector2 uv = Quasirandom::hammersley(i, 250, random);
					Vector4 direction = Quasirandom::uniformHemiSphere(uv, d);
					cl += cms->get(direction).saturated() * dot3(d, direction);
				}
				cl *= Scalar(PI);
				cl /= 250.0_simd;
				cmd->set(d, cl);
			}
		}
	}

	// Convert cube map to equirectangular image.
	Ref< drawing::Image > radiance = cmd->createEquirectangular();
	T_FATAL_ASSERT(radiance != nullptr);

	// Discard alpha channels as they are not used.
	radiance->clearAlpha(1.0);

	// Blur image slightly to reduce sampling speeks, do this in rectangular image to prevent cube leaks.
	Ref< drawing::ConvolutionFilter > blurFilter = drawing::ConvolutionFilter::createGaussianBlur(1);
	radiance->apply(blurFilter);

	// Create tracer environment.
	tracerTask->addTracerEnvironment(new TracerEnvironment(new IblProbe(radiance)));

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

	return alignUp(size, 16);
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
	model::CleanDuplicates(0.0f).apply(*model);
	model::CleanDegenerate().apply(*model);
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

/*! */
bool loadMaterialTextures(editor::IPipelineBuilder* pipelineBuilder, model::Model* model, const Guid& lightmapId, const std::wstring& assetPath)
{
	// Modify all materials to contain reference to lightmap channel.
	for (auto& material : model->getMaterials())
	{
		material.setBlendOperator(model::Material::BoDecal);
		material.setLightMap(model::Material::Map(L"Lightmap", L"Lightmap", false, lightmapId));
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

			Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + textureAsset->getFileName());
			Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
			if (!file)
				continue;

			Ref< drawing::Image > image = drawing::Image::load(file, textureAsset->getFileName().getExtension());
			if (!image)
				continue;

			diffuseMap.image = image;			
			material.setDiffuseMap(diffuseMap);
		}
	}
	return true;
}

/*! */
bool addModel(
	editor::IPipelineBuilder* pipelineBuilder,
	model::Model* model,
	const Transform& transform,
	const std::wstring& name,
	int32_t priority,
	const Guid& lightmapId,
	int32_t lightmapSize,
	TracerTask* tracerTask
)
{
	// Create output instance.
	if (pipelineBuilder->getOutputDatabase()->getInstance(lightmapId) == nullptr)
	{
		Ref< render::TextureResource > outputResource = new render::TextureResource();
		Ref< db::Instance > outputInstance = pipelineBuilder->getOutputDatabase()->createInstance(
			L"Generated/" + lightmapId.format(),
			db::CifReplaceExisting,
			&lightmapId
		);
		if (!outputInstance)
		{
			log::error << L"BakePipelineOperator failed; unable to create output instance." << Endl;
			return false;
		}

		outputInstance->setObject(outputResource);

		// Create output data stream.
		Ref< IStream > stream = outputInstance->writeData(L"Data");
		if (!stream)
		{
			log::error << L"BakePipelineOperator failed; unable to create texture data stream." << Endl;
			outputInstance->revert();
			return false;
		}

		const int32_t c_size = 64;
		const uint32_t c_white = 0xff333333;
		const uint32_t c_black = 0xff222222;

		Writer writer(stream);
		writer << uint32_t(12);
		writer << c_size;
		writer << c_size;
		writer << int32_t(1);
		writer << int32_t(1);
		writer << int32_t(render::TfR8G8B8A8);
		writer << bool(false);
		writer << uint8_t(render::Tt2D);
		writer << bool(false);
		writer << bool(false);

		for (int32_t y = 0; y < c_size; ++y)
		{
			for (int32_t x = 0; x < c_size; ++x)
			{
				uint32_t color = ((x / 8 + y / 8) & 1) ? c_white : c_black;
				if (writer.write(&color, 4, 1) != 4)
					return false;
			}
		}

		stream->close();

		if (!outputInstance->commit())
		{
			log::error << L"BakePipelineOperator failed; unable to commit output instance." << Endl;
			return false;
		}
	}

	tracerTask->addTracerModel(new TracerModel(
		model,
		transform
	));

	tracerTask->addTracerOutput(new TracerOutput(
		name,
		priority,
		model,
		transform,
		lightmapId,
		lightmapSize
	));

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.BakePipelineOperator", 0, BakePipelineOperator, scene::IScenePipelineOperator)

Ref< TracerProcessor > BakePipelineOperator::ms_tracerProcessor = nullptr;

BakePipelineOperator::BakePipelineOperator()
:	m_tracerType(nullptr)
,	m_editor(false)
{
}

bool BakePipelineOperator::create(const editor::IPipelineSettings* settings)
{
	// Check if baking is enabled, if not then we leave tracer type as null.
	if (!settings->getProperty< bool >(L"BakePipelineOperator.Enable", true))
		return true;

	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	m_modelCachePath = settings->getProperty< std::wstring >(L"Pipeline.ModelCache.Path", L"");

	std::wstring tracerTypeName = settings->getProperty< std::wstring >(L"BakePipelineOperator.RayTracerType", L"traktor.shape.RayTracerEmbree");
	if (tracerTypeName.empty())
		return false;

	m_tracerType = TypeInfo::find(tracerTypeName.c_str());
	if (!m_tracerType)
		return false;

	m_compressionMethod = settings->getProperty< std::wstring >(L"TexturePipeline.CompressionMethod", L"DXTn");
	m_editor = settings->getProperty< bool >(L"Pipeline.TargetEditor", false);

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

	// Create output path for debugging data.
	// FileSystem::getInstance().makeAllDirectories(Path(L"data/Temp/Bake"));
	return true;
}

void BakePipelineOperator::destroy()
{
}

TypeInfoSet BakePipelineOperator::getOperatorTypes() const
{
	return makeTypeInfoSet< BakeConfiguration >();
}

bool BakePipelineOperator::addDependencies(editor::IPipelineDepends* pipelineDepends, const ISerializable* operatorData, const scene::SceneAsset* sceneAsset) const
{
	pipelineDepends->addDependency< render::ShaderGraph >();

	// Add "use" dependencies to first level of external entity datas; so we ensure scene pipeline is invoked if external entity is modified.
	for (const auto layer : sceneAsset->getLayers())
	{
		scene::Traverser::visit(layer, [&](const world::EntityData* entityData) -> scene::Traverser::VisitorResult
		{
			if (auto editorAttributes = entityData->getComponent< world::EditorAttributesComponentData >())
			{
				if (!editorAttributes->include || editorAttributes->dynamic)
					return scene::Traverser::VrSkip;
			}

			const world::ExternalEntityData* externalEntityData = dynamic_type_cast< const world::ExternalEntityData* >(entityData);
			if (externalEntityData != nullptr)
				pipelineDepends->addDependency(externalEntityData->getEntityData(), editor::PdfUse);

			return scene::Traverser::VrContinue;
		});
	}

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
	if (!m_tracerType)
		return true;

	// In case no tracer processor is registered we create one for this build only,
	// by doing so we can ensure trace is finished before returning.
	Ref< TracerProcessor > tracerProcessor = ms_tracerProcessor;
	if (!tracerProcessor)
		tracerProcessor = new TracerProcessor(m_tracerType, pipelineBuilder->getOutputDatabase(), m_compressionMethod, true);

	// Cancel any bake process currently running for given scene.
	tracerProcessor->cancel(sourceInstance->getGuid());

	// Load last known receipt, used for prioritizing moved/new entities.
	Ref< BakeReceipt > receipt;
	if (m_editor)
	{
		Guid receiptId = sourceInstance->getGuid().permutation(1);
		if ((receipt = pipelineBuilder->getOutputDatabase()->getObjectReadOnly< BakeReceipt >(receiptId)) == nullptr)
			receipt = new BakeReceipt();
	}

	log::info << L"Creating lightmap tasks..." << Endl;

	Ref< TracerTask > tracerTask = new TracerTask(
		sourceInstance->getGuid(),
		configuration
	);

	RefArray< world::LayerEntityData > layers;

	// Find all static meshes and lights; replace external referenced entities with local if necessary.
	for (const auto layer : inoutSceneAsset->getLayers())
	{
		// Resolve all external entities, inital seed is null since we don't want to modify entity ID on those
		// entities which are inlines in scene, only those referenced from an external entity should be re-assigned IDs.
		Ref< world::LayerEntityData > flattenedLayer = checked_type_cast< world::LayerEntityData* >(resolveAllExternal(pipelineBuilder, layer, Guid::null));
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

		// Traverse and visit all entities in layer.
		scene::Traverser::visit(flattenedLayer, [&](Ref< world::EntityData >& inoutEntityData) -> scene::Traverser::VisitorResult
		{
			Guid entityId = inoutEntityData->getId();
			if (entityId.isNull())
				return scene::Traverser::VrSkip;

			// Check editor attributes component if we should include entity.
			if (auto editorAttributes = inoutEntityData->getComponent< world::EditorAttributesComponentData >())
			{
				if (!editorAttributes->include || editorAttributes->dynamic)
					return scene::Traverser::VrSkip;
			}

			// Add light source.
			if (auto lightComponentData = inoutEntityData->getComponent< world::LightComponentData >())
			{
				if (addLight(lightComponentData, inoutEntityData->getTransform(), tracerTask))
					inoutEntityData->removeComponent(lightComponentData);
			}

			// Add sky source.
			if (auto skyComponentData = inoutEntityData->getComponent< weather::SkyComponentData >())
				addSky(pipelineBuilder, m_assetPath, skyComponentData, tracerTask);

			// Calculate synthesized ids.
			Guid lightmapId = entityId.permutation(c_lightmapIdSeed);
			Guid outputId = entityId.permutation(c_outputIdSeed);

			// Find model synthesizer which can generate from current entity.
			const scene::IEntityReplicator* entityReplicator = m_entityReplicators[&type_of(inoutEntityData)];
			if (entityReplicator)
			{
				// Calculate hash of entity and it's dependencies; need to anonymize entity a bit since
				// cached product doesn't depend on id, name nor transform.
				uint32_t entityHash;
				{
					auto id = inoutEntityData->getId();
					inoutEntityData->setId(Guid::null);

					auto name = inoutEntityData->getName();
					inoutEntityData->setName(L"");

					auto transform = inoutEntityData->getTransform();
					inoutEntityData->setTransform(Transform::identity());

					entityHash = pipelineBuilder->calculateInclusiveHash(inoutEntityData);

					inoutEntityData->setId(id);
					inoutEntityData->setName(name);
					inoutEntityData->setTransform(transform);
				}

				Ref< model::Model > model = pipelineBuilder->getDataAccessCache()->read< model::Model >(
					entityHash,
					[&](IStream* stream) -> Ref< model::Model > {
						return BinarySerializer(stream).readObject< model::Model >();
					},
					[=](const model::Model* model, IStream* stream) -> bool {
						return BinarySerializer(stream).writeObject(model);
					},
					[&]() -> Ref< model::Model > {
						log::info << L"Preparing \"" << inoutEntityData->getName() << L"\" for tracing..." << Endl;

						Ref< model::Model > model = entityReplicator->createModel(pipelineBuilder, m_assetPath, inoutEntityData);
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
					return scene::Traverser::VrFailed;

				// Calculate size of lightmap from geometry.
				int32_t lightmapSize = calculateLightmapSize(
					model,
					configuration->getLumelDensity(),
					configuration->getMinimumLightMapSize(),
					configuration->getMaximumLightMapSize()
				);

				// Load model's material textures.
				if (!loadMaterialTextures(pipelineBuilder, model, lightmapId, m_assetPath))
					return scene::Traverser::VrFailed;

				// Calculate priority, if entity has moved since last bake then it's prioritized.
				int32_t priority = 0;
				if (receipt)
				{
					Transform lastTransform;
					if (receipt->getLastKnownTransform(entityId, lastTransform))
					{
						if (lastTransform != inoutEntityData->getTransform())
							priority = 1;
					}
					else
						priority = 1;

					receipt->setTransform(entityId, inoutEntityData->getTransform());
				}

				if (!addModel(
					pipelineBuilder,
					model,
					inoutEntityData->getTransform(),
					inoutEntityData->getName(),
					priority,
					lightmapId,
					lightmapSize,
					tracerTask
				))
					return scene::Traverser::VrFailed;

				// Let model generator consume altered model and modify entity in ways
				// which make sense for entity data.
				inoutEntityData = checked_type_cast< world::EntityData* >(entityReplicator->modifyOutput(
					pipelineBuilder,
					m_assetPath,
					inoutEntityData,
					model,
					outputId
				));

				// Skip further processing of this entity and it's children.
				return scene::Traverser::VrSkip;
			}

			// Find model synthesizer which can generate from components.
			RefArray< world::IEntityComponentData > componentDatas = inoutEntityData->getComponents();
			for (auto componentData : componentDatas)
			{
				const scene::IEntityReplicator* entityReplicator = m_entityReplicators[&type_of(componentData)];
				if (!entityReplicator)
					continue;

				uint32_t componentDataHash = pipelineBuilder->calculateInclusiveHash(componentData);

				Ref< model::Model > model = pipelineBuilder->getDataAccessCache()->read< model::Model >(
					componentDataHash,
					[&](IStream* stream) -> Ref< model::Model > {
						return BinarySerializer(stream).readObject< model::Model >();
					},
					[=](const model::Model* model, IStream* stream) -> bool {
						return BinarySerializer(stream).writeObject(model);
					},
					[&]() -> Ref< model::Model > {
						Ref< model::Model > model = entityReplicator->createModel(pipelineBuilder, m_assetPath, componentData);
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
					return scene::Traverser::VrFailed;

				// Calculate size of lightmap from geometry.
				int32_t lightmapSize = calculateLightmapSize(
					model,
					configuration->getLumelDensity(),
					configuration->getMinimumLightMapSize(),
					configuration->getMaximumLightMapSize()
				);

				// Load model's material textures.
				if (!loadMaterialTextures(pipelineBuilder, model, lightmapId, m_assetPath))
					return scene::Traverser::VrFailed;

				// Calculate priority, if entity has moved since last bake then it's prioritized.
				int32_t priority = 0;
				if (receipt)
				{
					Transform lastTransform;
					if (receipt->getLastKnownTransform(entityId, lastTransform))
					{
						if (lastTransform != inoutEntityData->getTransform())
							priority = 1;
					}
					else
						priority = 1;

					receipt->setTransform(entityId, inoutEntityData->getTransform());
				}

				if (!addModel(
					pipelineBuilder,
					model,
					inoutEntityData->getTransform(),
					inoutEntityData->getName(),
					priority,
					lightmapId,
					lightmapSize,
					tracerTask
				))
					return scene::Traverser::VrFailed;

				// Let model generator consume altered model and modify entity in ways
				// which make sense for entity data.
				Ref< world::IEntityComponentData > replaceComponentData = checked_type_cast< world::IEntityComponentData* >(entityReplicator->modifyOutput(
					pipelineBuilder,
					m_assetPath,
					componentData,
					model,
					outputId
				));
				if (replaceComponentData == nullptr)
					inoutEntityData->removeComponent(componentData);
				else if (replaceComponentData != componentData)
				{
					inoutEntityData->removeComponent(componentData);
					inoutEntityData->setComponent(replaceComponentData);
				}
			}

			return scene::Traverser::VrContinue;
		});

		// Replace with modified layer in output scene.
		layers.push_back(flattenedLayer);
	}
	inoutSceneAsset->setLayers(layers);

	// Create irradiance grid task.
	{
		const Guid c_irradianceGridIdSeed(L"{714D9AF6-EF62-4E15-B372-7CEBB090417B}");
		Guid irradianceGridId = sourceInstance->getGuid().permutation(c_irradianceGridIdSeed);

		// Create a black irradiance grid first.
		if (pipelineBuilder->getOutputDatabase()->getInstance(irradianceGridId) == nullptr)
		{
			Ref< world::IrradianceGridResource > outputResource = new world::IrradianceGridResource();
			Ref< db::Instance > outputInstance = pipelineBuilder->getOutputDatabase()->createInstance(
				L"Generated/" + irradianceGridId.format(),
				db::CifReplaceExisting,
				&irradianceGridId
			);
			if (!outputInstance)
			{
				log::error << L"BakePipelineOperator failed; unable to create output instance." << Endl;
				return false;
			}

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
		}

		tracerTask->addTracerIrradiance(new TracerIrradiance(
			L"Irradiance",
			irradianceGridId,
			Aabb3()
		));

		// Modify scene with our generated irradiance grid resource.
		inoutSceneAsset->getWorldRenderSettings()->irradianceGrid = resource::Id< world::IrradianceGrid >(
			irradianceGridId
		);
	}

	// Finally enqueue task to tracer processor.
	tracerProcessor->enqueue(tracerTask);

	// If we're not running with an external processor then we need to wait.
	if (!ms_tracerProcessor)
	{
		log::info << L"Waiting for lightmap baking to complete..." << Endl;
		tracerProcessor->waitUntilIdle();
		tracerProcessor = nullptr;
	}

	// Write out the receipt.
	if (receipt)
	{
		Guid receiptId = sourceInstance->getGuid().permutation(1);
		Ref< db::Instance > receiptInstance = pipelineBuilder->getOutputDatabase()->createInstance(L"Generated/" + receiptId.format(), db::CifReplaceExisting, &receiptId);
		if (receiptInstance)
		{
			receiptInstance->setObject(receipt);
			receiptInstance->commit();
		}
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
