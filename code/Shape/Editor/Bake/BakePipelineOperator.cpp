#include <functional>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Winding3.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IPipelineBuilder.h"
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
#include "Render/Resource/TextureResource.h"
#include "Scene/Editor/SceneAsset.h"
#include "Shape/Editor/IModelGenerator.h"
#include "Shape/Editor/Traverser.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/BakePipelineOperator.h"
#include "Shape/Editor/Bake/TracerIrradiance.h"
#include "Shape/Editor/Bake/TracerLight.h"
#include "Shape/Editor/Bake/TracerModel.h"
#include "Shape/Editor/Bake/TracerOutput.h"
#include "Shape/Editor/Bake/TracerProcessor.h"
#include "Shape/Editor/Bake/TracerTask.h"
#include "World/WorldRenderSettings.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/LightComponentData.h"

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

/*! Add light to tracer scene. */
void addLight(const world::LightComponentData* lightComponentData, const Transform& transform, TracerTask* tracerTask)
{
	Light light;
	if (lightComponentData->getLightType() == world::LtDirectional)
	{
		light.type = Light::LtDirectional;
		light.position = Vector4::origo();
		light.direction = -transform.axisY();
		light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
		light.range = Scalar(0.0f);
		tracerTask->addTracerLight(new TracerLight(light));
	}
	else if (lightComponentData->getLightType() == world::LtPoint)
	{
		light.type = Light::LtPoint;
		light.position = transform.translation().xyz1();
		light.direction = Vector4::zero();
		light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
		light.range = Scalar(lightComponentData->getRange());
		tracerTask->addTracerLight(new TracerLight(light));
	}
	else if (lightComponentData->getLightType() == world::LtSpot)
	{
		light.type = Light::LtSpot;
		light.position = transform.translation().xyz1();
		light.direction = -transform.axisY();
		light.color = Color4f(lightComponentData->getColor());
		light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
		light.range = Scalar(lightComponentData->getRange());
		light.radius = Scalar(lightComponentData->getRadius());
		tracerTask->addTracerLight(new TracerLight(light));
	}
	else if (lightComponentData->getLightType() != world::LtProbe)
		log::warning << L"BakePipelineOperator warning; unsupported light type of light." << Endl;
}

/*! */
bool addModel(const model::Model* model, const Transform& transform, const std::wstring& name, const Guid& lightmapId, int32_t lightmapSize, editor::IPipelineBuilder* pipelineBuilder, TracerTask* tracerTask)
{
	Ref< model::Model > mutableModel = DeepClone(model).create< model::Model >();
	if (!mutableModel)
	{
		log::error << L"BakePipelineOperator failed; unable to clone model." << Endl;
		return false;
	}

	// Create output instance.
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
int32_t calculateLightmapSize(const model::Model* model, float lumelDensity, int32_t minimumSize)
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
	const float size = std::sqrt(totalLightMapArea);
        
	return alignUp(std::max< int32_t >(minimumSize, (int32_t)(size + 0.5f)), 16);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.BakePipelineOperator", 0, BakePipelineOperator, scene::IScenePipelineOperator)

Ref< TracerProcessor > BakePipelineOperator::ms_tracerProcessor = nullptr;

BakePipelineOperator::BakePipelineOperator()
:	m_tracerType(nullptr)
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

	// Create instances of all concrete model generators.
	TypeInfoSet modelGeneratorTypes;
	type_of< IModelGenerator >().findAllOf(modelGeneratorTypes, false);
	for (const auto& modelGeneratorType : modelGeneratorTypes)
	{
		Ref< IModelGenerator > modelGenerator = dynamic_type_cast< IModelGenerator* >(modelGeneratorType->createInstance());
		if (modelGenerator)
			m_modelGenerators.push_back(modelGenerator);
	}
	
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

bool BakePipelineOperator::build(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* operatorData,
	const db::Instance* sourceInstance,
	scene::SceneAsset* inoutSceneAsset
) const
{
	Ref< TracerProcessor > tracerProcessor = ms_tracerProcessor;

	// In case no tracer processor is registered we create one for this build only,
	// by doing so we can ensure trace is finished before returning.
	if (!tracerProcessor)
		tracerProcessor = new TracerProcessor(m_tracerType, pipelineBuilder->getOutputDatabase());

	const auto configuration = mandatory_non_null_type_cast< const BakeConfiguration* >(operatorData);
	uint32_t configurationHash = DeepHash(configuration).get();
	Guid seedId = configuration->getSeedGuid();

	Ref< TracerTask > tracerTask = new TracerTask(
		sourceInstance->getGuid(),
		configuration
	);

	// Find all static meshes and lights; replace external referenced entities with local if necessary.
	RefArray< world::LayerEntityData > layers;
	for (const auto layer : inoutSceneAsset->getLayers())
	{
		if (!(layer->isInclude() && !layer->isDynamic()))
		{
			seedId.permutate();
			layers.push_back(layer);
			continue;
		}

		// Resolve all external entities.
		Ref< world::LayerEntityData > flattenedLayer = checked_type_cast< world::LayerEntityData* >(resolveAllExternal(pipelineBuilder, layer));
		if (!flattenedLayer)
			return false;

		// Calculate hash of current layer.
		int32_t layerHash = configurationHash + (int32_t)DeepHash(flattenedLayer).get();

		// Check if layer has already been baked, hash is written as a receipt in output database.
		Guid existingLayerHashId = seedId.permutate();

		Ref< PropertyInteger > existingLayerHash = pipelineBuilder->getOutputDatabase()->getObjectReadOnly< PropertyInteger >(existingLayerHashId);
		if (existingLayerHash && *existingLayerHash == layerHash)
		{
			log::info << L"Skipping baking lightmap, already baked in output database." << Endl;
			layers.push_back(flattenedLayer);
			continue;
		}

		// Either hash doesn't match or no receipt exist, create new receipt.
		Ref< db::Instance > hashInstance = pipelineBuilder->getOutputDatabase()->createInstance(
			L"Generated/" + existingLayerHashId.format(),
			db::CifReplaceExisting | db::CifKeepExistingGuid,
			&existingLayerHashId
		);
		hashInstance->setObject(new PropertyInteger(layerHash));
		hashInstance->commit();

		// Traverse and visit all entities in layer.
		Traverser(flattenedLayer).visit([&](Ref< world::EntityData >& inoutEntityData) -> bool
		{
			if (auto componentEntityData = dynamic_type_cast< world::ComponentEntityData* >(inoutEntityData))
			{
				if (auto lightComponentData = componentEntityData->getComponent< world::LightComponentData >())
				{
					addLight(lightComponentData, inoutEntityData->getTransform(), tracerTask);

					// Remove this light when we're tracing direct lighting.
					if (configuration->traceDirect())
						componentEntityData->removeComponent(lightComponentData);
				}

				RefArray< world::IEntityComponentData > componentDatas = componentEntityData->getComponents();
				for (auto componentData : componentDatas)
				{
					// Find model synthesizer which can generate from current entity.
					const IModelGenerator* modelGenerator = findModelGenerator(type_of(componentData));
					if (!modelGenerator)
						continue;

					// Synthesize a model which we can trace.
					Ref< model::Model > model = modelGenerator->createModel(pipelineBuilder, m_assetPath, componentData);
					if (!model)
						continue;

					Guid lightmapId = seedId.permutate();

					// Ensure model is fit for tracing.
					model->clear(model::Model::CfColors | model::Model::CfJoints);
					model::Triangulate().apply(*model);
					model::CleanDuplicates(0.0f).apply(*model);
					model::CleanDegenerate().apply(*model);
					model::CalculateTangents().apply(*model);

					// check if model already contain lightmap UV or if we need to unwrap.
					bool shouldUnwrap = false;

					uint32_t channel = model->getTexCoordChannel(L"Lightmap");
					if (channel == model::c_InvalidIndex)
					{
						// No lightmap UV channel, need to add and unwrap automatically.
						channel = model->addUniqueTexCoordChannel(L"Lightmap");
						shouldUnwrap = true;
					}

					int32_t lightmapSize = calculateLightmapSize(
						model,
						configuration->getLumelDensity(),
						configuration->getMinimumLightMapSize()
					);

					if (shouldUnwrap)
					{
						T_FATAL_ASSERT(channel != model::c_InvalidIndex);
						model::UnwrapUV(channel, lightmapSize).apply(*model);
					}

					// Modify all materials to contain reference to lightmap channel.
					AlignedVector< model::Material > materials = model->getMaterials();
					for (auto& material : materials)
					{
						material.setBlendOperator(model::Material::BoDecal);
						material.setLightMap(model::Material::Map(L"__Illumination__", channel, false), 1.0f);
					}
					model->setMaterials(materials);

					// Write model for debugging into temporary folder.
					model::ModelFormat::writeAny(L"data/Temp/Bake/" + inoutEntityData->getName() + L".tmd", model);

					// Add model to raytracing task.
					if (!addModel(
						model,
						inoutEntityData->getTransform(),
						inoutEntityData->getName(),
						lightmapId,
						lightmapSize,
						pipelineBuilder,
						tracerTask
					))
						continue;

					// Let model generator consume altered model and modify entity in ways
					// which make sense for entity data.
					Ref< world::IEntityComponentData > replaceComponentData = checked_type_cast< world::IEntityComponentData* >(modelGenerator->modifyOutput(
						pipelineBuilder,
						m_assetPath,
						componentData,
						lightmapId,
						model
					));
					if (replaceComponentData == nullptr)
						componentEntityData->removeComponent(componentData);
					else if (replaceComponentData != componentData)
					{
						componentEntityData->removeComponent(componentData);
						componentEntityData->setComponent(replaceComponentData);
					}
				}
			}
			else	// non-component entity type.
			{
				Guid lightmapId = seedId.permutate();

				// Find model synthesizer which can generate from current entity.
				const IModelGenerator* modelGenerator = findModelGenerator(type_of(inoutEntityData));
				if (!modelGenerator)
					return true;

				// Synthesize a model which we can trace.
				Ref< model::Model > model = modelGenerator->createModel(pipelineBuilder, m_assetPath, inoutEntityData);
				if (!model)
					return true;

				// Ensure model is fit for tracing.
				model->clear(model::Model::CfColors | model::Model::CfJoints);
				model::Triangulate().apply(*model);
				model::CleanDuplicates(0.0f).apply(*model);
				model::CleanDegenerate().apply(*model);
				model::CalculateTangents().apply(*model);

				// check if model already contain lightmap UV or if we need to unwrap.
				bool shouldUnwrap = false;

				uint32_t channel = model->getTexCoordChannel(L"Lightmap");
				if (channel == model::c_InvalidIndex)
				{
					// No lightmap UV channel, need to add and unwrap automatically.
					channel = model->addUniqueTexCoordChannel(L"Lightmap");
					shouldUnwrap = true;
				}

				int32_t lightmapSize = calculateLightmapSize(
					model,
					configuration->getLumelDensity(),
					configuration->getMinimumLightMapSize()
				);

				if (shouldUnwrap)
				{
					T_FATAL_ASSERT(channel != model::c_InvalidIndex);
					model::UnwrapUV(channel, lightmapSize).apply(*model);
				}

				// Modify all materials to contain reference to lightmap channel.
				AlignedVector< model::Material > materials = model->getMaterials();
				for (auto& material : materials)
				{
					material.setBlendOperator(model::Material::BoDecal);
					material.setLightMap(model::Material::Map(L"__Illumination__", channel, false), 1.0f);
				}
				model->setMaterials(materials);

				// Write model for debugging into temporary folder.
				model::ModelFormat::writeAny(L"data/Temp/Bake/" + inoutEntityData->getName() + L".tmd", model);

				// Add model to raytracing task.
				if (!addModel(
					model,
					Transform::identity(),
					inoutEntityData->getName(),
					lightmapId,
					lightmapSize,
					pipelineBuilder,
					tracerTask
				))
					return false;

				// Let model generator consume altered model and modify entity in ways
				// which make sense for entity data.
				inoutEntityData = checked_type_cast< world::EntityData* >(modelGenerator->modifyOutput(
					pipelineBuilder,
					m_assetPath,
					inoutEntityData,
					lightmapId,
					model
				));
			}
			return true;
		});

		layers.push_back(flattenedLayer);
	}
	inoutSceneAsset->setLayers(layers);

	// Create irradiance grid task.
	if (configuration->traceIrradiance())
	{
		Guid irradianceGridId = seedId.permutate();

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

const IModelGenerator* BakePipelineOperator::findModelGenerator(const TypeInfo& sourceType) const
{
	for (auto modelGenerator : m_modelGenerators)
	{
		auto supportedTypes = modelGenerator->getSupportedTypes();
		if (supportedTypes.find(&sourceType) != supportedTypes.end())
			return modelGenerator;
	}
	return nullptr;
}

	}
}
