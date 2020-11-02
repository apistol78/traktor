#include <limits>
#include <functional>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Range.h"
#include "Core/Math/Winding3.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
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
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Model/Model.h"
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

/*! Resolve external entities, ie flatten scene without external references. */
Ref< ISerializable > resolveAllExternal(editor::IPipelineCommon* pipeline, const ISerializable* object)
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

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveAllExternal(pipeline, externalEntityData));
			if (!resolvedEntityData)
				return nullptr;

			resolvedEntityData->setName(externalEntityDataRef->getName());
			resolvedEntityData->setTransform(externalEntityDataRef->getTransform());

			objectMember->set(resolvedEntityData);
		}
		else if (objectMember->get())
		{
			objectMember->set(resolveAllExternal(pipeline, objectMember->get()));
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
	Ref< IStream > file = pipelineBuilder->openFile(filePath);
	if (!file)
		return;

	Ref< drawing::Image > skyImage = drawing::Image::load(file, textureAsset->getFileName().getExtension());
	if (!skyImage)
		return;

	safeClose(file);

	// Measure max intensity.
	//Scalar maxIntensity = 0.0_simd;
	//for (int32_t y = 0; y < skyImage->getHeight(); ++y)
	//{
	//	for (int32_t x = 0; x < skyImage->getWidth(); ++x)
	//	{
	//		Color4f cl;
	//		skyImage->getPixelUnsafe(x, y, cl);
	//		Scalar intensity = dot3(cl, Vector4(1.0f, 1.0f, 1.0f, 0.0f));
	//		maxIntensity = max(maxIntensity, intensity);
	//	}
	//}
	//if (maxIntensity <= 0.0_simd)
	//	return;

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

	// Convert cube map to equirectangular image.
	Ref< drawing::Image > radiance = render::CubeMap::createFromImage(skyImage)->createEquirectangular();
	T_FATAL_ASSERT(radiance != nullptr);

	// Discard alpha channels as they are not used.
	radiance->clearAlpha(1.0);

	// Blur image slightly to reduce sampling speeks, do this in rectangular image to prevent cube leaks.
	Ref< drawing::ConvolutionFilter > blurFilter = drawing::ConvolutionFilter::createGaussianBlur(4);
	radiance->apply(blurFilter);

	// Renormalize intensity of probe to ensure overall energy level is preserved.
	//Scalar maxIntensity2 = 0.0_simd;
	//for (int32_t y = 0; y < radiance->getHeight(); ++y)
	//{
	//	for (int32_t x = 0; x < radiance->getWidth(); ++x)
	//	{
	//		Color4f cl;
	//		radiance->getPixelUnsafe(x, y, cl);
	//		Scalar intensity = dot3(cl, Vector4(1.0f, 1.0f, 1.0f, 0.0f));
	//		maxIntensity2 = max(maxIntensity2, intensity);
	//	}
	//}
	//Scalar normIntensity = maxIntensity / maxIntensity2;
	//drawing::TransformFilter normFilter(Color4f(normIntensity, normIntensity, normIntensity, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));
	//radiance->apply(&normFilter);

	// Save debug copy of sky IBL radiance probe.
	radiance->save(L"data/Temp/Bake/SkyRadiance.png");

	// Create tracer environment.
	tracerTask->addTracerEnvironment(new TracerEnvironment(new IblProbe(radiance)));
}

/*! */
bool addModel(
	editor::IPipelineBuilder* pipelineBuilder,
	model::Model* model,
	const Transform& transform,
	const std::wstring& name,
	const Guid& lightmapId,
	int32_t lightmapSize,
	TracerTask* tracerTask
)
{
	Ref< model::Model > mutableModel = model; // DeepClone(model).create< model::Model >();
	if (!mutableModel)
	{
		log::error << L"BakePipelineOperator failed; unable to clone model." << Endl;
		return false;
	}

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

		Writer writer(stream);
		writer << uint32_t(12);
		writer << int32_t(1);
		writer << int32_t(1);
		writer << int32_t(1);
		writer << int32_t(1);
		writer << int32_t(render::TfR8G8B8A8);
		writer << bool(false);
		writer << uint8_t(render::Tt2D);
		writer << bool(false);
		writer << bool(false);

		uint32_t c_white = 0xfffffff;

		if (writer.write(&c_white, 4, 1) != 4)
			return false;

		stream->close();

		if (!outputInstance->commit())
		{
			log::error << L"BakePipelineOperator failed; unable to commit output instance." << Endl;
			return false;
		}
	}

	tracerTask->addTracerModel(new TracerModel(
		mutableModel,
		transform
	));

	tracerTask->addTracerOutput(new TracerOutput(
		name,
		0,
		mutableModel,
		transform,
		lightmapId,
		lightmapSize
	));

	return true;
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
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	
	std::wstring tracerTypeName = settings->getProperty< std::wstring >(L"BakePipelineOperator.RayTracerType", L"traktor.shape.RayTracerEmbree");
	if (tracerTypeName.empty())
		return false;

	m_tracerType = TypeInfo::find(tracerTypeName.c_str());
	if (!m_tracerType)
		return false;

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
	FileSystem::getInstance().makeAllDirectories(Path(L"data/Temp/Bake"));
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
	Ref< TracerProcessor > tracerProcessor = ms_tracerProcessor;

	// In case no tracer processor is registered we create one for this build only,
	// by doing so we can ensure trace is finished before returning.
	if (!tracerProcessor)
		tracerProcessor = new TracerProcessor(m_tracerType, pipelineBuilder->getOutputDatabase(), false);

	const auto configuration = mandatory_non_null_type_cast< const BakeConfiguration* >(operatorData);
	uint32_t configurationHash = DeepHash(configuration).get();

	Guid layerHashSeedId = pipelineBuilder->synthesizeOutputGuid(100000);
	Guid lightmapSeedId = pipelineBuilder->synthesizeOutputGuid(100000);
	Guid irradianceGridSeedId = pipelineBuilder->synthesizeOutputGuid(100000);

	Ref< TracerTask > tracerTask = new TracerTask(
		sourceInstance->getGuid(),
		configuration
	);

	RefArray< world::LayerEntityData > layers;

	// Find all static meshes and lights; replace external referenced entities with local if necessary.
	for (const auto layer : inoutSceneAsset->getLayers())
	{
		if (!(layer->isInclude() && !layer->isDynamic()))
		{
			layers.push_back(layer);
			continue;
		}

		// Resolve all external entities.
		Ref< world::LayerEntityData > flattenedLayer = checked_type_cast< world::LayerEntityData* >(resolveAllExternal(pipelineBuilder, layer));
		if (!flattenedLayer)
			return false;

		// // Calculate hash of current layer along with hash of trace configuration.
		// int32_t layerHash = configurationHash + (int32_t)DeepHash(flattenedLayer).get();

		// // Check if layer has already been baked, hash is written as a receipt in output database.
		// Guid existingLayerId = layerHashSeedId.permutate();
		// Guid existingLayerHashId = layerHashSeedId.permutate();

		// if (m_editor && !rebuild)
		// {
		// 	Ref< PropertyInteger > existingLayerHash = pipelineBuilder->getOutputDatabase()->getObjectReadOnly< PropertyInteger >(existingLayerHashId);
		// 	if (
		// 		existingLayerHash &&
		// 		*existingLayerHash == layerHash
		// 	)
		// 	{
		// 		// Read flattened layer from output as we need the modified layer from last bake.
		// 		Ref< world::LayerEntityData > existingLayer = pipelineBuilder->getOutputDatabase()->getObjectReadOnly< world::LayerEntityData >(existingLayerId);
		// 		if (existingLayer)
		// 		{
		// 			log::info << L"Skipping baking lightmap, already baked in output database." << Endl;
		// 			layers.push_back(existingLayer);
		// 			continue;
		// 		}
		// 	}
		// }

		// Traverse and visit all entities in layer.
		int32_t debugIndex = 0;
		scene::Traverser::visit(flattenedLayer, [&](Ref< world::EntityData >& inoutEntityData) -> scene::Traverser::VisitorResult
		{
			if (auto lightComponentData = inoutEntityData->getComponent< world::LightComponentData >())
			{
				if (addLight(lightComponentData, inoutEntityData->getTransform(), tracerTask))
					inoutEntityData->removeComponent(lightComponentData);
			}

			if (auto skyComponentData = inoutEntityData->getComponent< weather::SkyComponentData >())
				addSky(pipelineBuilder, m_assetPath, skyComponentData, tracerTask);

			RefArray< world::IEntityComponentData > componentDatas = inoutEntityData->getComponents();
			for (auto componentData : componentDatas)
			{
				// Find model synthesizer which can generate from current entity.
				const scene::IEntityReplicator* entityReplicator = m_entityReplicators[&type_of(componentData)];
				if (!entityReplicator)
					continue;

				// Synthesize a model which we can trace.
				Ref< model::Model > model = entityReplicator->createModel(pipelineBuilder, m_assetPath, componentData);
				if (!model)
					continue;

				std::wstring name = str(L"%s_%d", inoutEntityData->getName().c_str(), debugIndex); debugIndex++;
				Guid lightmapId = lightmapSeedId.permutate();

				// Ensure model is fit for tracing.
				model->clear(model::Model::CfColors | model::Model::CfJoints);
				model::Triangulate().apply(*model);
				model::CleanDuplicates(0.0f).apply(*model);
				model::CleanDegenerate().apply(*model);
				model::CalculateTangents(false).apply(*model);

				// Calculate size of lightmap from geometry.
				int32_t lightmapSize = calculateLightmapSize(
					model,
					configuration->getLumelDensity(),
					configuration->getMinimumLightMapSize(),
					configuration->getMaximumLightMapSize()
				);

				// Check if model already contain lightmap UV or if we need to unwrap.
				uint32_t channel = model->getTexCoordChannel(L"Lightmap");
				if (channel == model::c_InvalidIndex)
				{
					// No lightmap UV channel, need to add and unwrap automatically.
					channel = model->addUniqueTexCoordChannel(L"Lightmap");
					model::UnwrapUV(channel, lightmapSize).apply(*model);
				}

				// Modify all materials to contain reference to lightmap channel.
				for (auto& material : model->getMaterials())
				{
					material.setBlendOperator(model::Material::BoDecal);
					material.setLightMap(model::Material::Map(L"Lightmap", L"Lightmap", false, lightmapId));
				}

				// Add model to raytracing task.
				if (!addModel(
					pipelineBuilder,
					model,
					inoutEntityData->getTransform(),
					name,
					lightmapId,
					lightmapSize,
					tracerTask
				))
					continue;

				// Let model generator consume altered model and modify entity in ways
				// which make sense for entity data.
				Ref< world::IEntityComponentData > replaceComponentData = checked_type_cast< world::IEntityComponentData* >(entityReplicator->modifyOutput(
					pipelineBuilder,
					m_assetPath,
					componentData,
					model
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

		// if (m_editor)
		// {
		// 	// Write baked layer and hash.
		// 	Ref< db::Instance > hashInstance = pipelineBuilder->getOutputDatabase()->createInstance(
		// 		L"Generated/" + existingLayerHashId.format(),
		// 		db::CifReplaceExisting,
		// 		&existingLayerHashId
		// 	);
		// 	hashInstance->setObject(new PropertyInteger(layerHash));
		// 	hashInstance->commit();

		// 	Ref< db::Instance > layerInstance = pipelineBuilder->getOutputDatabase()->createInstance(
		// 		L"Generated/" + existingLayerId.format(),
		// 		db::CifReplaceExisting,
		// 		&existingLayerId
		// 	);
		// 	layerInstance->setObject(flattenedLayer);
		// 	layerInstance->commit();
		// }

		// Replace with modified layer in output scene.
		layers.push_back(flattenedLayer);
	}
	inoutSceneAsset->setLayers(layers);

	// Create irradiance grid task.
	{
		Guid irradianceGridId = irradianceGridSeedId.permutate();

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
