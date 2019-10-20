#include <limits>
#include <functional>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Range.h"
#include "Core/Math/Winding3.h"
#include "Core/Misc/SafeDestroy.h"
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
#include "Drawing/Filters/ScaleFilter.h"
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
#include "Shape/Editor/Bake/TracerIrradiance.h"
#include "Shape/Editor/Bake/TracerLight.h"
#include "Shape/Editor/Bake/TracerModel.h"
#include "Shape/Editor/Bake/TracerOutput.h"
#include "Shape/Editor/Bake/TracerProcessor.h"
#include "Shape/Editor/Bake/TracerTask.h"
#include "World/IrradianceGridResource.h"
#include "World/WorldRenderSettings.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/ComponentEntityData.h"
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
		light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
		light.range = Scalar(lightComponentData->getRange());
		light.radius = Scalar(lightComponentData->getRadius());
		tracerTask->addTracerLight(new TracerLight(light));
	}
}

/*! */
void addSky(
	editor::IPipelineBuilder* pipelineBuilder,
	const std::wstring& assetPath,
	const weather::SkyComponentData* skyComponentData,
	TracerTask* tracerTask
)
{
	const int32_t c_importanceCellSize = 16;
	const int32_t c_minImportanceSamples = 1;
	const int32_t c_maxImportanceSamples = 20;

	// Extract reference to sky image from shader.
	Ref< const render::ShaderGraph > shader = pipelineBuilder->getObjectReadOnly< render::ShaderGraph >(
		skyComponentData->getShader()
	);
	if (!shader)
		return;

	RefArray< render::Texture > textureNodes;
	shader->findNodesOf< render::Texture >(textureNodes);
	auto it = std::find_if(textureNodes.begin(), textureNodes.end(), [&](render::Texture* textureNode) {
		return textureNode->getComment() == L"Tag_Sky";
	});
	if (it == textureNodes.end())
		return;

	const auto& textureId = (*it)->getExternal();
	Ref< const render::TextureAsset > textureAsset = pipelineBuilder->getObjectReadOnly< render::TextureAsset >(textureId);
	if (!textureAsset)
		return;

	Ref< IStream > file = pipelineBuilder->openFile(Path(assetPath), textureAsset->getFileName().getOriginal());
	if (!file)
		return;

	Ref< drawing::Image > skyImage = drawing::Image::load(file, textureAsset->getFileName().getExtension());
	if (!skyImage)
		return;

	safeClose(file);

	// Ensure source image is a multiple of cell size.
	drawing::ScaleFilter scaleFilter(
		alignUp(skyImage->getWidth(), c_importanceCellSize),
		alignUp(skyImage->getHeight(), c_importanceCellSize),
		drawing::ScaleFilter::MnAverage,
		drawing::ScaleFilter::MgLinear
	);
	skyImage->apply(&scaleFilter);

	// Convert cube map to equirectangular image.
	Ref< drawing::Image > radiance = render::CubeMap::createFromCrossImage(skyImage)->createEquirectangular();
	T_FATAL_ASSERT(radiance != nullptr);

	// Measure intensity range in radiance image.
	Range< Scalar > range(
		Scalar( std::numeric_limits< float >::max()),
		Scalar(-std::numeric_limits< float >::max())
	);
	for (int32_t y = 0; y < radiance->getHeight(); ++y)
	{
		for (int32_t x = 0; x < radiance->getWidth(); ++x)
		{
			Color4f cl;
			radiance->getPixelUnsafe(x, y, cl);
			Scalar intensity = dot3(cl, Vector4(1.0f, 1.0f, 1.0f, 0.0f));
			range.min = std::min(range.min, intensity);
			range.max = std::max(range.max, intensity);
		}
	}

	// Plot importance density/probability image.
	Ref< drawing::Image > importance = new drawing::Image(
		drawing::PixelFormat::getARGBF32(),
		radiance->getWidth() / c_importanceCellSize,
		radiance->getHeight() / c_importanceCellSize
	);
	int32_t totalSampleCount = 0;
	for (int32_t y = 0; y < importance->getHeight(); ++y)
	{
		for (int32_t x = 0; x < importance->getWidth(); ++x)
		{
			// Find maximum intensity in cell.
			Scalar maxIntensity(0.0f);
			for (int32_t cy = 0; cy < c_importanceCellSize; ++cy)
			{
				for (int32_t cx = 0; cx < c_importanceCellSize; ++cx)
				{
					Color4f cl;
					radiance->getPixelUnsafe(x * c_importanceCellSize + cx, y * c_importanceCellSize + cy, cl);
					Scalar intensity = dot3(cl, Vector4(1.0f, 1.0f, 1.0f, 0.0f));
					maxIntensity = std::max(maxIntensity, intensity);
				}
			}
			
			// Calculate importance, number of samples required in cell.
			Scalar k = (maxIntensity - range.min) / range.delta();
			int32_t samples = (int32_t)(c_minImportanceSamples + k * Scalar(c_maxImportanceSamples - c_minImportanceSamples));
			importance->setPixelUnsafe(x, y, Color4f(
				(float)samples,
				0.0f,
				0.0f,
				0.0f
			));
			totalSampleCount += samples;
		}
	}

	// Calculate the probability of each cell.
	for (int32_t y = 0; y < importance->getHeight(); ++y)
	{
		for (int32_t x = 0; x < importance->getWidth(); ++x)
		{
			Color4f cl;
			importance->getPixelUnsafe(x, y, cl);
			
			float samples = cl.getRed();
			cl.setRed(Scalar(samples / c_maxImportanceSamples));
			cl.setGreen(Scalar(importance->getWidth() * importance->getHeight() * samples / totalSampleCount));

			importance->setPixelUnsafe(x, y, cl);
		}
	}

	// Discard alpha channels as they are not used.
	radiance->clearAlpha(1.0);
	importance->clearAlpha(1.0f);

	// radiance->save(L"data/Temp/Bake/Radiance.png");
	// importance->save(L"data/Temp/Bake/Importance.png");

	// Create tracer light.
	Light light;
	light.type = Light::LtProbe;
	light.position = Vector4::origo();
	light.direction = Vector4::zero();
	light.color = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	light.range = Scalar(1e8f);
	light.radius = Scalar(1e8f);
	light.probe = new IblProbe(radiance, importance);
	tracerTask->addTracerLight(new TracerLight(light));
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
	scene::SceneAsset* inoutSceneAsset,
	bool rebuild
) const
{
	Ref< TracerProcessor > tracerProcessor = ms_tracerProcessor;

	// In case no tracer processor is registered we create one for this build only,
	// by doing so we can ensure trace is finished before returning.
	if (!tracerProcessor)
		tracerProcessor = new TracerProcessor(m_tracerType, pipelineBuilder->getOutputDatabase());

	const auto configuration = mandatory_non_null_type_cast< const BakeConfiguration* >(operatorData);
	uint32_t configurationHash = DeepHash(configuration).get();

	Guid layerHashSeedId = pipelineBuilder->synthesizeOutputGuid(100000);
	Guid lightmapSeedId = pipelineBuilder->synthesizeOutputGuid(100000);
	Guid irradianceGridSeedId = pipelineBuilder->synthesizeOutputGuid(100000);

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
			layers.push_back(layer);
			continue;
		}

		// Resolve all external entities.
		Ref< world::LayerEntityData > flattenedLayer = checked_type_cast< world::LayerEntityData* >(resolveAllExternal(pipelineBuilder, layer));
		if (!flattenedLayer)
			return false;

		// Calculate hash of current layer along with hash of trace configuration.
		int32_t layerHash = configurationHash + (int32_t)DeepHash(flattenedLayer).get();

		// Check if layer has already been baked, hash is written as a receipt in output database.
		Guid existingLayerId = layerHashSeedId.permutate();
		Guid existingLayerHashId = layerHashSeedId.permutate();

		if (m_editor && !rebuild)
		{
			Ref< PropertyInteger > existingLayerHash = pipelineBuilder->getOutputDatabase()->getObjectReadOnly< PropertyInteger >(existingLayerHashId);
			if (
				existingLayerHash &&
				*existingLayerHash == layerHash
			)
			{
				// Read flattened layer from output as we need the modified layer from last bake.
				Ref< world::LayerEntityData > existingLayer = pipelineBuilder->getOutputDatabase()->getObjectReadOnly< world::LayerEntityData >(existingLayerId);
				if (existingLayer)
				{
					log::info << L"Skipping baking lightmap, already baked in output database." << Endl;
					layers.push_back(existingLayer);
					continue;
				}
			}
		}

		// Traverse and visit all entities in layer.
		scene::Traverser::visit(flattenedLayer, [&](Ref< world::EntityData >& inoutEntityData) -> scene::Traverser::VisitorResult
		{
			if (auto componentEntityData = dynamic_type_cast< world::ComponentEntityData* >(inoutEntityData))
			{
				if (auto lightComponentData = componentEntityData->getComponent< world::LightComponentData >())
					addLight(lightComponentData, inoutEntityData->getTransform(), tracerTask);

				if (auto skyComponentData = componentEntityData->getComponent< weather::SkyComponentData >())
					addSky(pipelineBuilder, m_assetPath, skyComponentData, tracerTask);

				RefArray< world::IEntityComponentData > componentDatas = componentEntityData->getComponents();
				for (auto componentData : componentDatas)
				{
					// Find model synthesizer which can generate from current entity.
					Ref< const scene::IEntityReplicator > entityReplicator = scene::IEntityReplicator::createEntityReplicator(type_of(componentData));
					if (!entityReplicator)
						continue;

					// Synthesize a model which we can trace.
					Ref< model::Model > model = entityReplicator->createModel(pipelineBuilder, m_assetPath, componentData);
					if (!model)
						continue;

					Guid lightmapId = lightmapSeedId.permutate();

					// Ensure model is fit for tracing.
					model->clear(model::Model::CfColors | model::Model::CfJoints);
					model::Triangulate().apply(*model);
					model::CleanDuplicates(0.0f).apply(*model);
					model::CleanDegenerate().apply(*model);
					model::CalculateTangents(false).apply(*model);

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
						configuration->getMinimumLightMapSize(),
						configuration->getMaximumLightMapSize()
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
						material.setLightMap(model::Material::Map(L"Lightmap", channel, false, lightmapId));

						uint32_t flags = 0;
						if (configuration->traceDirect())
							flags |= model::Material::LmfRadiance;
						if (configuration->traceIndirect())
							flags |= model::Material::LmfIrradiance;
						material.setLightMapFlags(flags);					
					}
					model->setMaterials(materials);

					// Write model for debugging into temporary folder.
					// model::ModelFormat::writeAny(L"data/Temp/Bake/" + inoutEntityData->getName() + L".tmd", model);

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
					Ref< world::IEntityComponentData > replaceComponentData = checked_type_cast< world::IEntityComponentData* >(entityReplicator->modifyOutput(
						pipelineBuilder,
						m_assetPath,
						componentData,
						// lightmapId,
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
				Guid lightmapId = lightmapSeedId.permutate();

				// Find model synthesizer which can generate from current entity.
				Ref< const scene::IEntityReplicator > entityReplicator = scene::IEntityReplicator::createEntityReplicator(type_of(inoutEntityData));
				if (!entityReplicator)
					return scene::Traverser::VrContinue;

				// Synthesize a model which we can trace.
				Ref< model::Model > model = entityReplicator->createModel(pipelineBuilder, m_assetPath, inoutEntityData);
				if (!model)
					return scene::Traverser::VrFailed;

				// Ensure model is fit for tracing.
				model->clear(model::Model::CfColors | model::Model::CfJoints);
				model::Triangulate().apply(*model);
				model::CleanDuplicates(0.0f).apply(*model);
				model::CleanDegenerate().apply(*model);
				model::CalculateTangents(false).apply(*model);

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
					configuration->getMinimumLightMapSize(),
					configuration->getMaximumLightMapSize()
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
					material.setLightMap(model::Material::Map(L"Lightmap", channel, false, lightmapId));

					uint32_t flags = 0;
					if (configuration->traceDirect())
						flags |= model::Material::LmfRadiance;
					if (configuration->traceIndirect())
						flags |= model::Material::LmfIrradiance;
					material.setLightMapFlags(flags);
				}
				model->setMaterials(materials);

				// Write model for debugging into temporary folder.
				// model::ModelFormat::writeAny(L"data/Temp/Bake/" + inoutEntityData->getName() + L".tmd", model);

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
					return scene::Traverser::VrFailed;

				// Let model generator consume altered model and modify entity in ways
				// which make sense for entity data.
				inoutEntityData = checked_type_cast< world::EntityData* >(entityReplicator->modifyOutput(
					pipelineBuilder,
					m_assetPath,
					inoutEntityData,
					model
				));
			}
			return scene::Traverser::VrContinue;
		});

		if (m_editor)
		{
			// Write baked layer and hash.
			Ref< db::Instance > hashInstance = pipelineBuilder->getOutputDatabase()->createInstance(
				L"Generated/" + existingLayerHashId.format(),
				db::CifReplaceExisting,
				&existingLayerHashId
			);
			hashInstance->setObject(new PropertyInteger(layerHash));
			hashInstance->commit();

			Ref< db::Instance > layerInstance = pipelineBuilder->getOutputDatabase()->createInstance(
				L"Generated/" + existingLayerId.format(),
				db::CifReplaceExisting,
				&existingLayerId
			);
			layerInstance->setObject(flattenedLayer);
			layerInstance->commit();
		}

		// Replace with modified layer in output scene.
		layers.push_back(flattenedLayer);
	}
	inoutSceneAsset->setLayers(layers);

	// Create irradiance grid task.
	if (configuration->traceIrradiance())
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
