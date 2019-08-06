#include <functional>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
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
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/NormalizeTexCoords.h"
#include "Model/Operations/Transform.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/UnwrapUV.h"
#include "Render/Types.h"
#include "Render/Editor/Texture/CubeMap.h"
#include "Render/Editor/Texture/IrradianceProbeAsset.h"
#include "Render/Resource/TextureResource.h"
#include "Scene/Editor/SceneAsset.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/BakePipelineOperator.h"
#include "Shape/Editor/Bake/IblProbe.h"
#include "Shape/Editor/Bake/TracerLight.h"
#include "Shape/Editor/Bake/TracerModel.h"
#include "Shape/Editor/Bake/TracerOutput.h"
#include "Shape/Editor/Bake/TracerProcessor.h"
#include "Shape/Editor/Bake/TracerTask.h"
#include "Shape/Editor/Prefab/PrefabEntityData.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/CameraComponentData.h"
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/LightComponentData.h"
#include "World/Entity/ProbeComponentData.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

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

/*! Traverse data structure, visit each entity data.
 * \param object Current object in data structure.
 * \param visitor Function called for each found entity data, return true if should recurse further.
 */
void visit(ISerializable* object, const std::function< bool(world::EntityData*) >& visitor)
{
	Ref< Reflection > reflection = Reflection::create(object);

 	RefArray< ReflectionMember > objectMembers;
 	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	for (auto member : objectMembers)
	{
		RfmObject* objectMember = dynamic_type_cast< RfmObject* >(member);

		if (auto entityData = dynamic_type_cast< world::EntityData* >(objectMember->get()))
		{
			if (visitor(entityData))
				visit(entityData, visitor);
		}
		else if (objectMember->get())
			visit(objectMember->get(), visitor);
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.BakePipelineOperator", 0, BakePipelineOperator, scene::IScenePipelineOperator)

Ref< TracerProcessor > BakePipelineOperator::ms_tracerProcessor = nullptr;

BakePipelineOperator::BakePipelineOperator()
{
}

bool BakePipelineOperator::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
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
		tracerProcessor = new TracerProcessor(pipelineBuilder->getOutputDatabase());

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
		if (layer->isInclude() && !layer->isDynamic())
		{
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
			visit(flattenedLayer, [&](world::EntityData* entityData) -> bool
			{
				if (auto componentEntityData = dynamic_type_cast< world::ComponentEntityData* >(entityData))
				{
					if (auto lightComponentData = componentEntityData->getComponent< world::LightComponentData >())
					{
						// Add light to tracer scene.
						Light light;
						if (lightComponentData->getLightType() == world::LtDirectional)
						{
							light.type = Light::LtDirectional;
							light.position = Vector4::origo();
							light.direction = -entityData->getTransform().axisY();
							light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
							light.range = Scalar(0.0f);
							tracerTask->addTracerLight(new TracerLight(light));
						}
						else if (lightComponentData->getLightType() == world::LtPoint)
						{
							light.type = Light::LtPoint;
							light.position = entityData->getTransform().translation().xyz1();
							light.direction = Vector4::zero();
							light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
							light.range = Scalar(lightComponentData->getRange());
							tracerTask->addTracerLight(new TracerLight(light));
						}
						else if (lightComponentData->getLightType() == world::LtSpot)
						{
							light.type = Light::LtSpot;
							light.position = entityData->getTransform().translation().xyz1();
							light.direction = -entityData->getTransform().axisY();
							light.color = Color4f(lightComponentData->getColor());
							light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
							light.range = Scalar(lightComponentData->getRange());
							light.radius = Scalar(lightComponentData->getRadius());
							tracerTask->addTracerLight(new TracerLight(light));
						}
						else if (lightComponentData->getLightType() != world::LtProbe)
							log::warning << L"BakePipelineOperator warning; unsupported light type of light \"" << entityData->getName() << L"\"." << Endl;

						// Remove this light when we're tracing direct lighting.
						if (configuration->traceDirect())
							componentEntityData->removeComponent(lightComponentData);
					}

					if (auto meshComponentData = componentEntityData->getComponent< mesh::MeshComponentData >())
					{
						Ref< mesh::MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< mesh::MeshAsset >(
							meshComponentData->getMesh()
						);
						if (!meshAsset)
							return false;

						// \tbd We should probably ignore mesh assets with custom shaders.

						Ref< model::Model > model = model::ModelFormat::readAny(meshAsset->getFileName(), [&](const Path& p) {
							return pipelineBuilder->openFile(Path(m_assetPath), p.getOriginal());
						});
						if (!model)
							return false;

						// Transform model into world space.
						model::Transform(entityData->getTransform().toMatrix44()).apply(*model);

						// Reset transformation of entity.
						componentEntityData->setTransform(Transform::identity());

						uint32_t channel = model->getTexCoordChannel(L"Lightmap");

						// Create tracer model.						
						Ref< model::Model > tm = DeepClone(model).create< model::Model >();
						tm->clear(model::Model::CfColors | model::Model::CfJoints);
						model::Triangulate().apply(*tm);
						model::CleanDuplicates(0.001f).apply(*tm);
						model::CleanDegenerate().apply(*tm);
						model::CalculateTangents().apply(*tm);
						tracerTask->addTracerModel(new TracerModel(tm));

						// Create tracer output.
						Ref< model::Model > rm = DeepClone(model).create< model::Model >();
						rm->clear(model::Model::CfColors | model::Model::CfJoints);
						model::NormalizeTexCoords(channel, 1.0f / 16, 1.0f / 16).apply(*rm);
						model::CleanDuplicates(0.001f).apply(*rm);
						model::CleanDegenerate().apply(*rm);

						AlignedVector< model::Material > materials = rm->getMaterials();
						for (auto& material : materials)
						{
							material.setBlendOperator(model::Material::BoDecal);
							material.setLightMap(model::Material::Map(L"__Illumination__", channel, false), 1.0f);
						}
						rm->setMaterials(materials);

						Guid lightmapId = seedId.permutate();
						bool firstBake = false;

						// Create a dummy, white, output texture only if no previous lightmap exist.
						if (pipelineBuilder->getOutputDatabase()->getInstance(lightmapId) == nullptr)
						{
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

							firstBake = true;
						}

						tracerTask->addTracerOutput(new TracerOutput(
							entityData->getName(),
							firstBake ? 100 : 0,
							rm,
							lightmapId
						));

						// Create output mesh asset.
						auto materialTextures = meshAsset->getMaterialTextures();
						materialTextures[L"__Illumination__"] = lightmapId;

						Guid outputMeshId = seedId.permutate();
						
						Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
						outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
						outputMeshAsset->setMaterialTextures(materialTextures);

						pipelineBuilder->buildOutput(
							outputMeshAsset,
							L"Generated/" + outputMeshId.format(),
							outputMeshId,
							rm
						);

						// Modify component to reference our output mesh asset.
						meshComponentData->setMesh(resource::Id< mesh::IMesh >(
							outputMeshId
						));
					}

					// Get global IBL light from probe.
					if (auto probeComponentData = componentEntityData->getComponent< world::ProbeComponentData >())
					{
						if (!probeComponentData->getLocal())
						{
							Ref< const render::IrradianceProbeAsset > irradianceAsset = pipelineBuilder->getObjectReadOnly< render::IrradianceProbeAsset >(probeComponentData->getDiffuseTexture());
							if (irradianceAsset)
							{
								Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), irradianceAsset->getFileName().getOriginal());
								if (!file)
								{
									log::error << L"Probe texture asset pipeline failed; unable to open source image \"" << irradianceAsset->getFileName().getOriginal() << L"\"" << Endl;
									return false;
								}

								Ref< drawing::Image > assetImage = drawing::Image::load(file, irradianceAsset->getFileName().getExtension());
								if (!assetImage)
								{
									log::error << L"Probe texture asset pipeline failed; unable to load source image \"" << irradianceAsset->getFileName().getOriginal() << L"\"" << Endl;
									return false;
								}

								file->close();

								Light light;
								light.type = Light::LtProbe;
								light.probe = new IblProbe(
									new render::CubeMap(assetImage)
								);
								tracerTask->addTracerLight(new TracerLight(light));

								// Remove probe from scene as it's contribution will be baked.
								componentEntityData->removeComponent(probeComponentData);
							}
						}
					}

					// // Terrain
					// if (auto terrainComponentData = componentEntityData->getComponent< terrain::TerrainComponentData >())
					// {
					// 	const auto& terrain = terrainComponentData->getTerrain();

					// 	Ref< const terrain::TerrainAsset > terrainAsset = pipelineBuilder->getObjectReadOnly< terrain::TerrainAsset >(terrain);
					// 	if (!terrain)
					// 		return false;

					// 	Ref< db::Instance > heightfieldAssetInstance = pipelineBuilder->getSourceDatabase()->getInstance(terrainAsset->getHeightfield());
					// 	if (!heightfieldAssetInstance)
					// 		return false;

					// 	Ref< const hf::HeightfieldAsset > heightfieldAsset = heightfieldAssetInstance->getObject< const hf::HeightfieldAsset >();
					// 	if (!heightfieldAsset)
					// 		return false;

					// 	Ref< IStream > sourceData = heightfieldAssetInstance->readData(L"Data");
					// 	if (!sourceData)
					// 		return false;

					// 	Ref< hf::Heightfield > heightfield = hf::HeightfieldFormat().read(
					// 		sourceData,
					// 		heightfieldAsset->getWorldExtent()
					// 	);

					// 	safeClose(sourceData);

					// 	if (!heightfield)
					// 		return false;

					// 	Ref< model::Model > model = hf::ConvertHeightfield().convert(heightfield, 16, 0.0f);
					// 	if (!model)
					// 		return false;
					// }

					//if (auto cameraComponentData = componentEntityData->getComponent< world::CameraComponentData >())
					//{
					//	tracerTask->addTracerOutput(new TracerCameraOutput(
					//	));
					//}

					// Do not recurse further as we cannot make sure we know which
					// other kinds of components own untraceable entities.
					return false;
				}
				else if (auto prefabEntityData = dynamic_type_cast< PrefabEntityData* >(entityData))
				{
					RefArray< model::Model > models;
					std::map< std::wstring, Guid > materialTextures;

					// We have reached a prefab; collect all models and remove all mesh components from prefab.
					visit(prefabEntityData, [&](world::EntityData* entityData) -> bool
					{
						if (auto componentEntityData = dynamic_type_cast< world::ComponentEntityData* >(entityData))
						{
							if (auto meshComponentData = componentEntityData->getComponent< mesh::MeshComponentData >())
							{
								Ref< mesh::MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< mesh::MeshAsset >(
									meshComponentData->getMesh()
								);
								if (!meshAsset)
									return false;

								// \tbd We should probably ignore mesh assets with custom shaders.

								Ref< model::Model > model = model::ModelFormat::readAny(meshAsset->getFileName(), [&](const Path& p) {
									return pipelineBuilder->openFile(Path(m_assetPath), p.getOriginal());
								});
								if (!model)
									return false;

								// Transform model into world space.
								model::Transform(entityData->getTransform().toMatrix44()).apply(*model);

								model->clear(model::Model::CfColors | model::Model::CfJoints);
								models.push_back(model);

								materialTextures.insert(
									meshAsset->getMaterialTextures().begin(),
									meshAsset->getMaterialTextures().end()
								);

								componentEntityData->removeComponent(meshComponentData);
							}			
						}
						return true;
					});

					if (!models.empty())
					{
						// Calculate number of UV tiles.
						int32_t tiles = (int32_t)(std::ceil(std::sqrt(models.size())) + 0.5f);

						// Offset lightmap UV into tiles.
						for (int32_t i = 0; i < (int32_t)models.size(); ++i)
						{
							float tileU = (float)(i % tiles) / tiles;
							float tileV = (float)(i / tiles) / tiles;

							uint32_t channel = models[i]->getTexCoordChannel(L"Lightmap");
							if (channel != model::c_InvalidIndex)
							{
								// First normalize and add margin to model.
								model::NormalizeTexCoords(channel, 1.0f / 16, 1.0f / 16).apply(*models[i]);

								// Offset all texcoords into tile.
								AlignedVector< model::Vertex > vertices = models[i]->getVertices();
								for (auto& vertex : vertices)
								{
									Vector2 uv = models[i]->getTexCoord(vertex.getTexCoord(channel));
									uv *= (float)(1.0f / tiles);
									uv += Vector2(tileU, tileV);
									vertex.setTexCoord(channel, models[i]->addUniqueTexCoord(uv));
								}
								models[i]->setVertices(vertices);
							}
						}

						// Create merged model.
						Ref< model::Model > mergedModel = new model::Model();
						for (int32_t i = 0; i < (int32_t)models.size(); ++i)
						{
							model::CleanDuplicates(0.001f).apply(*models[i]);
							model::MergeModel(*models[i], Transform::identity(), 0.001f).apply(*mergedModel);
						}

						uint32_t channel = mergedModel->getTexCoordChannel(L"Lightmap");

						// Create tracer model.						
						Ref< model::Model > tm = DeepClone(mergedModel).create< model::Model >();
						model::Triangulate().apply(*tm);
						model::CleanDuplicates(0.001f).apply(*tm);
						model::CleanDegenerate().apply(*tm);
						model::CalculateTangents().apply(*tm);
						model::ModelFormat::writeAny(L"data/Temp/Bake/" + entityData->getName() + L"_Tracer.tmd", tm);
						tracerTask->addTracerModel(new TracerModel(tm));

						// Create tracer output.
						Ref< model::Model > rm = DeepClone(mergedModel).create< model::Model >();
						model::CleanDuplicates(0.001f).apply(*rm);
						model::CleanDegenerate().apply(*rm);
						model::ModelFormat::writeAny(L"data/Temp/Bake/" + entityData->getName() + L"_Visual.tmd", rm);

						AlignedVector< model::Material > materials = rm->getMaterials();
						for (auto& material : materials)
						{
							material.setBlendOperator(model::Material::BoDecal);
							material.setLightMap(model::Material::Map(L"__Illumination__", channel, false), 1.0f);
						}
						rm->setMaterials(materials);

						Guid lightmapId = seedId.permutate();
						bool firstBake = false;

						// Create a dummy, white, output texture only if no previous lightmap exist.
						if (pipelineBuilder->getOutputDatabase()->getInstance(lightmapId) == nullptr)
						{
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

							firstBake = true;
						}

						tracerTask->addTracerOutput(new TracerOutput(
							entityData->getName(),
							firstBake ? 100 : 0,
							rm,
							lightmapId
						));

						// Create output mesh asset.
						materialTextures[L"__Illumination__"] = lightmapId;

						Guid outputMeshId = seedId.permutate();

						Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
						outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
						outputMeshAsset->setMaterialTextures(materialTextures);

						pipelineBuilder->buildOutput(
							outputMeshAsset,
							L"Generated/" + outputMeshId.format(),
							outputMeshId,
							rm
						);						

						// Create a new child entity to layer (we cannot add to prefab) which contain reference to our merged visual mesh.
						Ref< mesh::MeshComponentData > meshComponentData = new mesh::MeshComponentData();
						meshComponentData->setMesh(resource::Id< mesh::IMesh >(
							outputMeshId
						));

						Ref< world::ComponentEntityData > mergedEntity = new world::ComponentEntityData();
						mergedEntity->setComponent(meshComponentData);
						flattenedLayer->addEntityData(mergedEntity);						
					}

					// As we have already taken care of prefabs's children we stop from recursing further.
					return false;
				}
				else
					return true;
			});

			layers.push_back(flattenedLayer);
		}
		else
		{
			// Permutate seed so we can ignore if layer inclusion flags change.
			seedId.permutate();

			// Add layer to output.
			layers.push_back(layer);
		}
	}
	inoutSceneAsset->setLayers(layers);

	tracerProcessor->enqueue(tracerTask);

	// Raytrace "ground truths" of each camera.
	// if (false)
	// {
	// 	for (uint32_t i = 0; i < cameraEntityDatas.size(); ++i)
	// 	{
	// 		auto cameraEntityData = cameraEntityDatas[i];
	// 		T_FATAL_ASSERT(cameraEntityData != nullptr);

	// 		auto cameraComponentData = cameraEntityData->getComponent< world::CameraComponentData >();
	// 		T_FATAL_ASSERT(cameraComponentData != nullptr);

	// 		if (cameraComponentData->getCameraType() != world::CtPerspective)
	// 			continue;

	// 		log::info << L"Tracing camera \"" << cameraEntityData->getName() << L"\" (" << i << L"/" << cameraEntityDatas.size() << L")..." << Endl;

	// 		Ref< drawing::Image > image = rayTracer->traceCamera(cameraEntityData->getTransform(), 1280, 720, cameraComponentData->getFieldOfView());
	// 		if (!image)
	// 			continue;

	// 		drawing::TonemapFilter tonemapFilter;
	// 		image->apply(&tonemapFilter);

	// 		drawing::GammaFilter gammaFilter(1.0f / 2.2f);
	// 		image->apply(&gammaFilter);

	// 		image->save(cameraEntityData->getName() + L"_" + toString(i) + L"_Camera.png");
	// 	}
	// }

	// Raytrace IBL probes.
	// for (uint32_t i = 0; i < lightEntityDatas.size(); ++i)
	// {
	// 	auto lightEntityData = lightEntityDatas[i];
	// 	T_FATAL_ASSERT(lightEntityData != nullptr);

	// 	auto lightComponentData = lightEntityData->getComponent< world::LightComponentData >();
	// 	T_FATAL_ASSERT(lightComponentData != nullptr);

	// 	if (lightComponentData->getLightType() != world::LtProbe)
	// 		continue;

	// 	log::info << L"Tracing SH probe \"" << lightEntityData->getName() << L"\" (" << i << L"/" << lightEntityDatas.size() << L")..." << Endl;

	// 	auto position = lightEntityData->getTransform().translation().xyz1();

	// 	Ref< render::SHCoeffs > shCoeffs = rayTracer->traceProbe(position);
	// 	if (shCoeffs)
	// 		lightComponentData->setSHCoeffs(shCoeffs);
	// }

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
