#include <functional>
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Float.h"
#include "Core/Math/Format.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Triangulator.h"
#include "Core/Math/Winding3.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Timer/Timer.h"
#include "Database/Database.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Filters/DilateFilter.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Filters/TonemapFilter.h"
#include "Drawing/Functions/BlendFunction.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/Transform.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/UnwrapUV.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Render/SH/SHEngine.h"
#include "Render/SH/SHFunction.h"
#include "Scene/Editor/SceneAsset.h"
#include "Shape/Editor/Bake/GBuffer.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/BakePipelineOperator.h"
#include "Shape/Editor/Bake/IRayTracer.h"
#include "Shape/Editor/Prefab/PrefabEntityData.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/CameraComponentData.h"
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/LightComponentData.h"

#if !defined(__RPI__) && !defined(__APPLE__)
#	include <OpenImageDenoise/oidn.h>
#endif

namespace traktor
{
	namespace shape
	{
		namespace
		{

struct BakeProcessMesh
{
	std::wstring name;
	Ref< model::Model > renderModel;	//!< Render model, using lightmap.
	Ref< model::Model > tracerModel;	//!< Tracer model, part of RT scene.
	Guid lightMapId;
	Ref< render::TextureOutput > lightMapTextureAsset;
	Guid meshId;
	Ref< mesh::MeshAsset > meshAsset;
};

Ref< ISerializable > resolveAllExternal(editor::IPipelineCommon* pipeline, const ISerializable* object)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< RfmObject > objectMembers;
	reflection->findMembers< RfmObject >(objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

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

// void collectTraceEntities(
// 	const ISerializable* object,
// 	RefArray< world::ComponentEntityData >& outLightEntityData,
// 	RefArray< world::ComponentEntityData >& outMeshEntityData,
// 	RefArray< world::ComponentEntityData >& outCameraEntityData
// )
// {
// 	Ref< Reflection > reflection = Reflection::create(object);

// 	RefArray< ReflectionMember > objectMembers;
// 	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

// 	while (!objectMembers.empty())
// 	{
// 		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
// 		objectMembers.pop_front();

// 		if (world::ComponentEntityData* componentEntityData = dynamic_type_cast< world::ComponentEntityData* >(objectMember->get()))
// 		{
// 			if (componentEntityData->getComponent< world::LightComponentData >() != nullptr)
// 				outLightEntityData.push_back(componentEntityData);
// 			if (componentEntityData->getComponent< mesh::MeshComponentData >() != nullptr)
// 				outMeshEntityData.push_back(componentEntityData);
// 			if (componentEntityData->getComponent< world::CameraComponentData >() != nullptr)
// 				outCameraEntityData.push_back(componentEntityData);
// 		}
// 		else if (objectMember->get())
// 			collectTraceEntities(objectMember->get(), outLightEntityData, outMeshEntityData, outCameraEntityData);
// 	}
// }

/*! Traverse data structure, visit each entity data.
 * \param object Current object in data structure.
 * \param visitor Function called for each found entity data, return true if should recurse further.
 */
void visit(ISerializable* object, const std::function< bool(world::EntityData*) >& visitor)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< RfmObject > objectMembers;
	reflection->findMembers< RfmObject >(objectMembers);

	for (auto objectMember : objectMembers)
	{
		if (auto entityData = dynamic_type_cast< world::EntityData* >(objectMember->get()))
		{
			if (visitor(entityData))
				visit(entityData, visitor);
		}
		else if (objectMember->get())
			visit(objectMember->get(), visitor);
	}
}

#if !defined(__RPI__) && !defined(__APPLE__)
Ref< drawing::Image > denoise(const GBuffer& gbuffer, drawing::Image* lightmap)
{
	int32_t width = lightmap->getWidth();
	int32_t height = lightmap->getHeight();

	lightmap->convert(drawing::PixelFormat::getRGBAF32());

	Ref< drawing::Image > albedo = new drawing::Image(
		drawing::PixelFormat::getRGBAF32(),
		lightmap->getWidth(),
		lightmap->getHeight()
	);
	albedo->clear(Color4f(1, 1, 1, 1));

	Ref< drawing::Image > normals = new drawing::Image(
		drawing::PixelFormat::getRGBAF32(),
		lightmap->getWidth(),
		lightmap->getHeight()
	);
	for (int32_t y = 0; y < height; ++y)
	{
		for (int32_t x = 0; x < width; ++x)
		{
			const auto elm = gbuffer.get(x, y);
			normals->setPixel(x, y, Color4f(elm.normal));
		}
	}

	Ref< drawing::Image > output = new drawing::Image(
		drawing::PixelFormat::getRGBAF32(),
		lightmap->getWidth(),
		lightmap->getHeight()
	);

	OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);
	oidnCommitDevice(device);

	OIDNFilter filter = oidnNewFilter(device, "RT"); // generic ray tracing filter
	oidnSetSharedFilterImage(filter, "color",  lightmap->getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0);
	oidnSetSharedFilterImage(filter, "albedo", albedo->getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0); // optional
	oidnSetSharedFilterImage(filter, "normal", normals->getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0); // optional
	oidnSetSharedFilterImage(filter, "output", output->getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0);
	oidnSetFilter1b(filter, "hdr", true); // image is HDR
	oidnCommitFilter(filter);

	oidnExecuteFilter(filter);	

	// Check for errors
	const char* errorMessage;
	if (oidnGetDeviceError(device, &errorMessage) != OIDN_ERROR_NONE)
		log::error << mbstows(errorMessage) << Endl;

	// Cleanup
	oidnReleaseFilter(filter);
	oidnReleaseDevice(device);	
	return output;
}
#endif

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.BakePipelineOperator", 0, BakePipelineOperator, scene::IScenePipelineOperator)

BakePipelineOperator::BakePipelineOperator()
:	m_rayTracerType(nullptr)
{
}

bool BakePipelineOperator::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	
	m_rayTracerType = TypeInfo::find(settings->getProperty< std::wstring >(L"BakePipelineOperator.RayTracerType", L"traktor.shape.RayTracerEmbree").c_str());
	if (!m_rayTracerType)
	{
		log::error << L"Failed to initialize shape pipeline operator; no such ray tracer type." << Endl;
		return false;
	}

	return true;
}

void BakePipelineOperator::destroy()
{
}

TypeInfoSet BakePipelineOperator::getOperatorTypes() const
{
	return makeTypeInfoSet< BakeConfiguration >();
}

bool BakePipelineOperator::build(editor::IPipelineBuilder* pipelineBuilder, const ISerializable* operatorData, scene::SceneAsset* inoutSceneAsset) const
{
	const auto configuration = mandatory_non_null_type_cast< const BakeConfiguration* >(operatorData);
	Guid seedId = configuration->getSeedGuid();

	// Create raytracer implementation.
	Ref< IRayTracer > rayTracer = checked_type_cast< IRayTracer* >(m_rayTracerType->createInstance());
	if (!rayTracer->create(configuration))
		return false;

	AlignedVector< BakeProcessMesh > processMeshes;

	// RefArray< world::ComponentEntityData > lightEntityDatas;
	// RefArray< world::ComponentEntityData > meshEntityDatas;
	// RefArray< world::ComponentEntityData > cameraEntityDatas;

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
							rayTracer->addLight(light);
						}
						else if (lightComponentData->getLightType() == world::LtPoint)
						{
							light.type = Light::LtPoint;
							light.position = entityData->getTransform().translation().xyz1();
							light.direction = Vector4::zero();
							light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
							light.range = Scalar(lightComponentData->getRange());
							rayTracer->addLight(light);
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
							rayTracer->addLight(light);
						}
						else if (lightComponentData->getLightType() != world::LtProbe)
							log::warning << L"IlluminateEntityPipeline warning; unsupported light type of light \"" << entityData->getName() << L"\"." << Endl;

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

						uint32_t channel = model->getTexCoordChannel(L"Lightmap");

						auto& processMesh = processMeshes.push_back();
						processMesh.name = entityData->getName();

						// Create render model.
						processMesh.renderModel = DeepClone(model).create< model::Model >();
						AlignedVector< model::Material > materials = processMesh.renderModel->getMaterials();
						for (auto& material : materials)
						{
							material.setBlendOperator(model::Material::BoDecal);
							material.setLightMap(model::Material::Map(L"__Illumination__", channel, false), 1.0f);
						}
						processMesh.renderModel->setMaterials(materials);

						// Create tracer model.						
						processMesh.tracerModel = DeepClone(model).create< model::Model >();
						processMesh.tracerModel->clear(model::Model::CfColors | model::Model::CfJoints);
						model::Triangulate().apply(*processMesh.tracerModel);
						model::CleanDuplicates(0.001f).apply(*processMesh.tracerModel);
						model::CleanDegenerate().apply(*processMesh.tracerModel);
						model::CalculateTangents().apply(*processMesh.tracerModel);
						rayTracer->addModel(processMesh.tracerModel, Transform::identity());

						// Create lightmap texture.
						processMesh.lightMapId = seedId.permutate();
						processMesh.lightMapTextureAsset = new render::TextureOutput();
						processMesh.lightMapTextureAsset->m_textureFormat = render::TfR16G16B16A16F;
						processMesh.lightMapTextureAsset->m_keepZeroAlpha = false;
						processMesh.lightMapTextureAsset->m_hasAlpha = false;
						processMesh.lightMapTextureAsset->m_ignoreAlpha = true;
						processMesh.lightMapTextureAsset->m_linearGamma = true;
						processMesh.lightMapTextureAsset->m_enableCompression = false;
						processMesh.lightMapTextureAsset->m_sharpenRadius = 0;
						processMesh.lightMapTextureAsset->m_systemTexture = true;
						processMesh.lightMapTextureAsset->m_generateMips = false;

						// Create output mesh asset.
						auto materialTextures = meshAsset->getMaterialTextures();
						materialTextures[L"__Illumination__"] = processMesh.lightMapId;

						processMesh.meshId = seedId.permutate();
						processMesh.meshAsset = new mesh::MeshAsset();
						processMesh.meshAsset->setMeshType(mesh::MeshAsset::MtStatic);
						processMesh.meshAsset->setMaterialTextures(materialTextures);

						// Modify component to reference our output mesh asset.
						meshComponentData->setMesh(resource::Id< mesh::IMesh >(
							processMesh.meshId
						));
					}

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
						int32_t tiles = (int32_t)(std::sqrt(models.size()) + 0.5f);

						// Offset lightmap UV into tiles.
						for (int32_t i = 0; i < (int32_t)models.size(); ++i)
						{
							float tileU = (float)(i % tiles) / tiles;
							float tileV = (float)(i / tiles) / tiles;

							uint32_t channel = models[i]->getTexCoordChannel(L"Lightmap");
							if (channel != model::c_InvalidIndex)
							{
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

						auto& processMesh = processMeshes.push_back();
						processMesh.name = entityData->getName();

						// Create merged model.
						Ref< model::Model > mergedModel = new model::Model();
						for (int32_t i = 0; i < (int32_t)models.size(); ++i)
						{
							model::CleanDuplicates(0.01f).apply(*models[i]);
							model::MergeModel(*models[i], Transform::identity(), 0.01f).apply(*mergedModel);
						}

						uint32_t channel = mergedModel->getTexCoordChannel(L"Lightmap");

						// Create render model.
						processMesh.renderModel = DeepClone(mergedModel).create< model::Model >();
						AlignedVector< model::Material > materials = processMesh.renderModel->getMaterials();
						for (auto& material : materials)
						{
							material.setBlendOperator(model::Material::BoDecal);
							material.setLightMap(model::Material::Map(L"__Illumination__", channel, false), 1.0f);
						}
						processMesh.renderModel->setMaterials(materials);

						// Create tracer model.						
						processMesh.tracerModel = DeepClone(mergedModel).create< model::Model >();
						processMesh.tracerModel->clear(model::Model::CfColors | model::Model::CfJoints);
						model::Triangulate().apply(*processMesh.tracerModel);
						model::CleanDuplicates(0.001f).apply(*processMesh.tracerModel);
						model::CleanDegenerate().apply(*processMesh.tracerModel);
						model::CalculateTangents().apply(*processMesh.tracerModel);
						rayTracer->addModel(processMesh.tracerModel, Transform::identity());

						// Create lightmap texture.
						processMesh.lightMapId = seedId.permutate();
						processMesh.lightMapTextureAsset = new render::TextureOutput();
						processMesh.lightMapTextureAsset->m_textureFormat = render::TfR16G16B16A16F;
						processMesh.lightMapTextureAsset->m_keepZeroAlpha = false;
						processMesh.lightMapTextureAsset->m_hasAlpha = false;
						processMesh.lightMapTextureAsset->m_ignoreAlpha = true;
						processMesh.lightMapTextureAsset->m_linearGamma = true;
						processMesh.lightMapTextureAsset->m_enableCompression = false;
						processMesh.lightMapTextureAsset->m_sharpenRadius = 0;
						processMesh.lightMapTextureAsset->m_systemTexture = true;
						processMesh.lightMapTextureAsset->m_generateMips = false;

						// Create output mesh asset.
						materialTextures[L"__Illumination__"] = processMesh.lightMapId;

						processMesh.meshId = seedId.permutate();
						processMesh.meshAsset = new mesh::MeshAsset();
						processMesh.meshAsset->setMeshType(mesh::MeshAsset::MtStatic);
						processMesh.meshAsset->setMaterialTextures(materialTextures);

						// Create a new child entity to prefab which contain reference to our merged visual mesh.
						Ref< mesh::MeshComponentData > meshComponentData = new mesh::MeshComponentData();
						meshComponentData->setMesh(resource::Id< mesh::IMesh >(
							processMesh.meshId
						));

						Ref< world::ComponentEntityData > mergedEntity = new world::ComponentEntityData();
						mergedEntity->setComponent(meshComponentData);
						prefabEntityData->addEntityData(mergedEntity);						
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
			layers.push_back(layer);
	}
	inoutSceneAsset->setLayers(layers);

	// Commit all lights and models; after this point
	// no more lights nor models can be added to tracer.
	rayTracer->commit();

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

	// Raytrace lightmap for each mesh.
	GBuffer gbuffer;
	for (uint32_t i = 0; i < processMeshes.size(); ++i)
	{
		auto renderModel = processMeshes[i].renderModel;

		// Calculate output size from lumel density.
		float totalWorldArea = 0.0f;
		for (const auto& polygon : renderModel->getPolygons())
		{
			Winding3 polygonWinding;
			for (const auto index : polygon.getVertices())
				polygonWinding.push(renderModel->getVertexPosition(index));
			totalWorldArea += abs(polygonWinding.area());
		}

		const float totalLightMapArea = configuration->getLumelDensity() * configuration->getLumelDensity() * totalWorldArea;
		const float size = std::sqrt(totalLightMapArea);
		
		const int32_t outputSize = alignUp(std::max< int32_t >(
			configuration->getMinimumLightMapSize(),
			 (int32_t)(size + 0.5f)
		), 16);

		// Unwrap lightmap UV.
		uint32_t channel = 0;
		// if (configuration->getEnableAutoTexCoords())
		// {
		// 	channel = model->addUniqueTexCoordChannel(L"Illuminate_LightmapUV");
		// 	if (!model::UnwrapUV(channel, outputSize).apply(*model))
		// 	{
		// 		log::error << L"IlluminateEntityPipeline failed; unable to unwrap UV of model \"" << meshEntityData->getName() << L"\"." << Endl;
		// 		return false;
		// 	}
		// }
		// else
		// {
		 	channel = renderModel->getTexCoordChannel(L"Lightmap");
		// 	if (channel == model::c_InvalidIndex)
		// 	{
		// 		log::warning << L"IlluminateEntityPipeline warning; no uv channel named \"Lightmap\" found, using channel 0." << Endl;
		// 		channel = 0;
		// 	}
		// }

		Timer timer;
		timer.start();

		// Create GBuffer of mesh's geometry.
		gbuffer.create(outputSize, outputSize, *renderModel, Transform::identity(), channel);
		// gbuffer.saveAsImages(meshEntityData->getName() + L"_" + toString(i) + L"_GBuffer_Pre");

		double TendGBuffer = timer.getElapsedTime();
		timer.start();

		// Preprocess GBuffer.
		rayTracer->preprocess(&gbuffer);
		// gbuffer.saveAsImages(meshEntityData->getName() + L"_" + toString(i) + L"_GBuffer_Post");

		double TendPreProcess = timer.getElapsedTime();
		timer.start();

		Ref< drawing::Image > lightmapDirect;
		if (configuration->traceDirect())
			lightmapDirect = rayTracer->traceDirect(&gbuffer);

		Ref< drawing::Image > lightmapIndirect;
		if (configuration->traceIndirect())
			lightmapIndirect = rayTracer->traceIndirect(&gbuffer);

		double TendTrace = timer.getElapsedTime();
		timer.start();

		if (configuration->getEnableDilate())
		{
			// Dilate lightmap to prevent leaking.
			drawing::DilateFilter dilateFilter(3);
			if (lightmapDirect)
				lightmapDirect->apply(&dilateFilter);
			if (lightmapIndirect)
				lightmapIndirect->apply(&dilateFilter);
		}

		// Blur indirect lightmap to reduce noise from path tracing.
		if (configuration->getEnableDenoise())
		{
#if !defined(__RPI__) && !defined(__APPLE__)
			if (lightmapDirect)
				lightmapDirect = denoise(gbuffer, lightmapDirect);
			if (lightmapIndirect)
				lightmapIndirect = denoise(gbuffer, lightmapIndirect);
#endif
		}

		// Merge direct and indirect lightmaps.
		Ref< drawing::Image > lightmap = new drawing::Image(drawing::PixelFormat::getRGBAF32(), outputSize, outputSize);
		lightmap->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

		if (lightmapDirect)
			lightmap->copy(lightmapDirect, 0, 0, outputSize, outputSize, drawing::BlendFunction(
				drawing::BlendFunction::BfOne,
				drawing::BlendFunction::BfOne,
				drawing::BlendFunction::BoAdd
			));

		if (lightmapIndirect)
			lightmap->copy(lightmapIndirect, 0, 0, outputSize, outputSize, drawing::BlendFunction(
				drawing::BlendFunction::BfOne,
				drawing::BlendFunction::BfOne,
				drawing::BlendFunction::BoAdd
			));

		lightmapDirect = nullptr;
		lightmapIndirect = nullptr;

		// Clamp shadow below threshold; to prevent tonemap to bring up noise.
		if (configuration->getClampShadowThreshold() > FUZZY_EPSILON)
		{
			for (uint32_t y = 0; y < lightmap->getHeight(); ++y)
			{
				for (uint32_t x = 0; x < lightmap->getWidth(); ++x)
				{
					Color4f lumel;
					lightmap->getPixelUnsafe(x, y, lumel);

					Scalar intensity = dot3(lumel, Vector4(1.0f, 1.0f, 1.0f, 0.0f));

					intensity = (intensity - Scalar(configuration->getClampShadowThreshold())) / Scalar(1.0f - configuration->getClampShadowThreshold());
					if (intensity < 0.0f)
						intensity = Scalar(0.0f);

					lightmap->setPixelUnsafe(x, y, lumel * intensity);
				}
			}
		}

		// Discard alpha.
		lightmap->clearAlpha(1.0f);

		double TendFilter = timer.getElapsedTime();
		timer.start();

		//lightmap->save(meshEntityData->getName() + L"_" + toString(i) + L"_Lightmap.png");
		//model::ModelFormat::writeAny(meshEntityData->getName() + L"_" + toString(i) + L"_Unwrapped.tmd", model);

		pipelineBuilder->buildOutput(
			processMeshes[i].lightMapTextureAsset,
			L"Generated/" + processMeshes[i].lightMapId.format(),
			processMeshes[i].lightMapId,
			lightmap
		);

		pipelineBuilder->buildOutput(
			processMeshes[i].meshAsset,
			L"Generated/" + processMeshes[i].meshId.format(),
			processMeshes[i].meshId,
			processMeshes[i].renderModel
		);

		double TendWrite = timer.getElapsedTime();
		timer.start();

		log::debug << L"Lightmap time breakdown;" << Endl;
		log::debug << L"  gbuffer    " << int32_t(TendGBuffer * 1000.0) << L" ms." << Endl;
		log::debug << L"  preprocess " << int32_t(TendPreProcess * 1000.0) << L" ms." << Endl;
		log::debug << L"  trace      " << int32_t(TendTrace * 1000.0) << L" ms." << Endl;
		log::debug << L"  filter     " << int32_t((TendFilter) * 1000.0) << L" ms." << Endl;
		log::debug << L"  output     " << int32_t((TendWrite) * 1000.0) << L" ms." << Endl;
	}

	return true;
}

	}
}
