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
#include "Illuminate/Editor/GBuffer.h"
#include "Illuminate/Editor/IlluminateConfiguration.h"
#include "Illuminate/Editor/IlluminatePipelineOperator.h"
#include "Illuminate/Editor/RayTracerEmbree.h"
#include "Illuminate/Editor/RayTracerLocal.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/UnwrapUV.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Render/SH/SHEngine.h"
#include "Render/SH/SHFunction.h"
#include "Scene/Editor/SceneAsset.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/CameraComponentData.h"
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/LightComponentData.h"

#include <OpenImageDenoise/oidn.h>

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

Ref< ISerializable > resolveAllExternal(editor::IPipelineCommon* pipeline, const ISerializable* object)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

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

void collectTraceEntities(
	const ISerializable* object,
	RefArray< world::ComponentEntityData >& outLightEntityData,
	RefArray< world::ComponentEntityData >& outMeshEntityData,
	RefArray< world::ComponentEntityData >& outCameraEntityData
)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (world::ComponentEntityData* componentEntityData = dynamic_type_cast< world::ComponentEntityData* >(objectMember->get()))
		{
			if (componentEntityData->getComponent< world::LightComponentData >() != nullptr)
				outLightEntityData.push_back(componentEntityData);
			if (componentEntityData->getComponent< mesh::MeshComponentData >() != nullptr)
				outMeshEntityData.push_back(componentEntityData);
			if (componentEntityData->getComponent< world::CameraComponentData >() != nullptr)
				outCameraEntityData.push_back(componentEntityData);
		}
		else if (objectMember->get())
			collectTraceEntities(objectMember->get(), outLightEntityData, outMeshEntityData, outCameraEntityData);
	}
}

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

void lineTraverse(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const std::function< void(int32_t, int32_t) >& fn)
{
	int32_t dx = x1 - x0;
	int32_t dy = y1 - y0;
	int32_t x = x0;
	int32_t y = y0;
	int32_t p = 2 * dy - dx;
	while (x < x1)
	{
		if (p >= 0)
		{
			fn(x, y);
			y++;
			p = p + 2 * dy - 2 * dx;
		}
		else
		{
			fn(x, y);
			p = p + 2 * dy;
		}
		++x;
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.illuminate.IlluminatePipelineOperator", 0, IlluminatePipelineOperator, scene::IScenePipelineOperator)

IlluminatePipelineOperator::IlluminatePipelineOperator()
:	m_rayTracerType(nullptr)
{
}

bool IlluminatePipelineOperator::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	
	m_rayTracerType = TypeInfo::find(settings->getProperty< std::wstring >(L"IlluminatePipelineOperator.RayTracerType", L"traktor.illuminate.RayTracerEmbree").c_str());
	if (!m_rayTracerType)
	{
		log::error << L"Failed to initialize illuminate pipeline operator; no such ray tracer type." << Endl;
		return false;
	}

	return true;
}

void IlluminatePipelineOperator::destroy()
{
}

TypeInfoSet IlluminatePipelineOperator::getOperatorTypes() const
{
	return makeTypeInfoSet< IlluminateConfiguration >();
}

bool IlluminatePipelineOperator::build(editor::IPipelineBuilder* pipelineBuilder, const ISerializable* operatorData, scene::SceneAsset* inoutSceneAsset) const
{
	const auto configuration = mandatory_non_null_type_cast< const IlluminateConfiguration* >(operatorData);

	// Create raytracer implementation.
	Ref< IRayTracer > rayTracer = checked_type_cast< IRayTracer* >(m_rayTracerType->createInstance());
	if (!rayTracer->create(configuration))
		return false;

	RefArray< world::ComponentEntityData > lightEntityDatas;
	RefArray< world::ComponentEntityData > meshEntityDatas;
	RefArray< world::ComponentEntityData > cameraEntityDatas;

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

			// Get all trace entities.
			collectTraceEntities(flattenedLayer, lightEntityDatas, meshEntityDatas, cameraEntityDatas);

			layers.push_back(flattenedLayer);
		}
		else
			layers.push_back(layer);
	}
	inoutSceneAsset->setLayers(layers);

	for (const auto lightEntityData : lightEntityDatas)
	{
		world::LightComponentData* lightComponentData = lightEntityData->getComponent< world::LightComponentData >();
		T_FATAL_ASSERT(lightComponentData != nullptr);

		Light light;
		if (lightComponentData->getLightType() == world::LtDirectional)
		{
			light.type = Light::LtDirectional;
			light.position = Vector4::origo();
			light.direction = -lightEntityData->getTransform().axisY();
			light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
			light.range = Scalar(0.0f);
			rayTracer->addLight(light);
		}
		else if (lightComponentData->getLightType() == world::LtPoint)
		{
			light.type = Light::LtPoint;
			light.position = lightEntityData->getTransform().translation().xyz1();
			light.direction = Vector4::zero();
			light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
			light.range = Scalar(lightComponentData->getRange());
			rayTracer->addLight(light);
		}
		else if (lightComponentData->getLightType() == world::LtSpot)
		{
			light.type = Light::LtSpot;
			light.position = lightEntityData->getTransform().translation().xyz1();
			light.direction = -lightEntityData->getTransform().axisY();
			light.color = Color4f(lightComponentData->getColor());
			light.color = lightComponentData->getColor() * Scalar(lightComponentData->getIntensity());
			light.range = Scalar(lightComponentData->getRange());
			light.radius = Scalar(lightComponentData->getRadius());
			rayTracer->addLight(light);
		}
		else if (lightComponentData->getLightType() != world::LtProbe)
			log::warning << L"IlluminateEntityPipeline warning; unsupported light type of light \"" << lightEntityData->getName() << L"\"." << Endl;

		// Disable all dynamic lights in scene if we're tracing direct lighting also.
		if (configuration->traceDirect())
			lightComponentData->setIntensity(0.0f);
	}

	RefArray< mesh::MeshAsset > meshAssets;
	RefArray< model::Model > models;

	for (const auto meshEntityData : meshEntityDatas)
	{
		Ref< mesh::MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< mesh::MeshAsset >(
			meshEntityData->getComponent< mesh::MeshComponentData >()->getMesh()
		);
		if (!meshAsset)
			continue;

		Ref< model::Model > model = model::ModelFormat::readAny(meshAsset->getFileName(), [&](const Path& p) {
			return pipelineBuilder->openFile(Path(m_assetPath), p.getOriginal());
		});
		if (!model)
		{
			log::warning << L"IlluminateEntityPipeline warning; unable to read model \"" << meshAsset->getFileName().getOriginal() << L"\"." << Endl;
			continue;
		}

		meshAssets.push_back(meshAsset);
		models.push_back(DeepClone(model).create< model::Model >());

		model->clear(model::Model::CfColors | model::Model::CfTexCoords | model::Model::CfJoints);
		model::Triangulate().apply(*model);
		model::CleanDuplicates(0.001f).apply(*model);
		model::CleanDegenerate().apply(*model);
		model::CalculateTangents().apply(*model);

		rayTracer->addModel(model, meshEntityData->getTransform());
	}

	// Commit all lights and models; after this point
	// no more lights nor models can be added to tracer.
	rayTracer->commit();

	// Raytrace "ground truths" of each camera.
	if (false)
	{
		for (uint32_t i = 0; i < cameraEntityDatas.size(); ++i)
		{
			auto cameraEntityData = cameraEntityDatas[i];
			T_FATAL_ASSERT(cameraEntityData != nullptr);

			auto cameraComponentData = cameraEntityData->getComponent< world::CameraComponentData >();
			T_FATAL_ASSERT(cameraComponentData != nullptr);

			if (cameraComponentData->getCameraType() != world::CtPerspective)
				continue;

			log::info << L"Tracing camera \"" << cameraEntityData->getName() << L"\" (" << i << L"/" << cameraEntityDatas.size() << L")..." << Endl;

			Ref< drawing::Image > image = rayTracer->traceCamera(cameraEntityData->getTransform(), 1280, 720, cameraComponentData->getFieldOfView());
			if (!image)
				continue;

			drawing::TonemapFilter tonemapFilter;
			image->apply(&tonemapFilter);

			drawing::GammaFilter gammaFilter(1.0f / 2.2f);
			image->apply(&gammaFilter);

			image->save(cameraEntityData->getName() + L"_" + toString(i) + L"_Camera.png");
		}
	}

	// Raytrace IBL probes.
	for (uint32_t i = 0; i < lightEntityDatas.size(); ++i)
	{
		auto lightEntityData = lightEntityDatas[i];
		T_FATAL_ASSERT(lightEntityData != nullptr);

		auto lightComponentData = lightEntityData->getComponent< world::LightComponentData >();
		T_FATAL_ASSERT(lightComponentData != nullptr);

		if (lightComponentData->getLightType() != world::LtProbe)
			continue;

		log::info << L"Tracing SH probe \"" << lightEntityData->getName() << L"\" (" << i << L"/" << lightEntityDatas.size() << L")..." << Endl;

		auto position = lightEntityData->getTransform().translation().xyz1();

		Ref< render::SHCoeffs > shCoeffs = rayTracer->traceProbe(position);
		if (shCoeffs)
			lightComponentData->setSHCoeffs(shCoeffs);
	}

	// Raytrace lightmap for each mesh.
	GBuffer gbuffer;
	for (uint32_t i = 0; i < meshEntityDatas.size(); ++i)
	{
		auto meshEntityData = meshEntityDatas[i];
		T_FATAL_ASSERT(meshEntityData != nullptr);

		log::info << L"Tracing lightmap \"" << meshEntityData->getName() << L"\" (" << i << L"/" << meshEntityDatas.size() << L")..." << Endl;

		Ref< model::Model > model = models[i];
		model::Triangulate().apply(*model);

		// Calculate output size from lumel density.
		float totalWorldArea = 0.0f;
		for (const auto& polygon : model->getPolygons())
		{
			Winding3 polygonWinding;
			for (const auto index : polygon.getVertices())
				polygonWinding.push(model->getVertexPosition(index));
			totalWorldArea += abs(polygonWinding.area());
		}

		float totalLightMapArea = configuration->getLumelDensity() * configuration->getLumelDensity() * totalWorldArea;
		float size = std::sqrt(totalLightMapArea);

		int32_t outputSize = alignUp(std::max< int32_t >(configuration->getMinimumLightMapSize(), (int32_t)(size + 0.5f)), 16);
		log::info << L"Lumel density " << configuration->getLumelDensity() << L" lumels/unit => lightmap size " << outputSize << Endl;

		// Unwrap lightmap UV.
		uint32_t channel = 0;
		if (configuration->getEnableAutoTexCoords())
		{
			channel = model->addUniqueTexCoordChannel(L"Illuminate_LightmapUV");
			if (!model::UnwrapUV(channel, outputSize).apply(*model))
			{
				log::error << L"IlluminateEntityPipeline failed; unable to unwrap UV of model \"" << meshEntityData->getName() << L"\"." << Endl;
				return false;
			}
		}
		else
		{
			channel = model->getTexCoordChannel(L"Lightmap");
			if (channel == model::c_InvalidIndex)
			{
				log::warning << L"IlluminateEntityPipeline warning; no uv channel named \"Lightmap\" found, using channel 0." << Endl;
				channel = 0;
			}
		}

		Timer timer;
		timer.start();

		// Create GBuffer of mesh's geometry.
		gbuffer.create(outputSize, outputSize, *model, meshEntityData->getTransform(), channel);
		gbuffer.saveAsImages(meshEntityData->getName() + L"_" + toString(i) + L"_GBuffer_Pre");

		double TendGBuffer = timer.getElapsedTime();
		timer.start();

		// Preprocess GBuffer.
		rayTracer->preprocess(&gbuffer);
		gbuffer.saveAsImages(meshEntityData->getName() + L"_" + toString(i) + L"_GBuffer_Post");

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

		/*
		// Discontinuity filtering.
		model::ModelAdjacency adjacency(model, model::ModelAdjacency::MdByPosition);
		for (auto polygon : model->getPolygons())
		for (uint32_t i = 0; i < model->getPolygonCount(); ++i)
		{
			auto polygon = model->getPolygon(i);

			for (uint32_t j = 0; j < polygon.getVertexCount(); ++j)
			{
				uint32_t halfEdge = adjacency.getEdge(i, j);

				AlignedVector< uint32_t > sharedEdges;
				adjacency.getSharedEdges(halfEdge, sharedEdges);
				if (sharedEdges.empty())
					continue;

				auto vtxA = model->getVertex(polygon.getVertex(j));
				auto vtxB = model->getVertex(polygon.getVertex((j + 1) % polygon.getVertexCount()));

				Vector2 sourceTexCoordA = model->getTexCoord(vtxA.getTexCoord(channel));
				Vector2 sourceTexCoordB = model->getTexCoord(vtxB.getTexCoord(channel));

				int32_t sx = (int32_t)std::floor(sourceTexCoordA.x + 0.5f);
				int32_t sy = (int32_t)std::floor(sourceTexCoordA.y + 0.5f);

				int32_t dx = (int32_t)std::floor(sourceTexCoordB.x + 0.5f);
				int32_t dy = (int32_t)std::floor(sourceTexCoordB.y + 0.5f);

				for (auto sharedEdge : sharedEdges)
				{
					auto sharedPolygon = model->getPolygon(adjacency.getPolygon(sharedEdge));
					auto sharedPolygonEdge = adjacency.getPolygonEdge(sharedEdge);

					auto sharedVtxA = model->getVertex(sharedPolygon.getVertex(sharedPolygonEdge));
					auto sharedVtxB = model->getVertex(sharedPolygon.getVertex((sharedPolygonEdge + 1) % sharedPolygon.getVertexCount()));

					Vector2 destTexCoordA = model->getTexCoord(sharedVtxA.getTexCoord(channel));
					Vector2 destTexCoordB = model->getTexCoord(sharedVtxB.getTexCoord(channel));

					// \tbd Need line traversal function.


					lineTraverse(
						sx, sy,
						dx, dy,
						[&](int32_t x, int32_t y)
						{
							//float fx = (x - sx);
							//float fy = (y - sy);
							//float f = std::sqrt(fx * fx + fy * fy);

							lightmapIndirect->setPixel(x, y, Color4f(1, 0, 0, 1));
						}
					);

				}
			}
		}
		*/

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
			if (lightmapDirect)
				lightmapDirect = denoise(gbuffer, lightmapDirect);
			if (lightmapIndirect)
				lightmapIndirect = denoise(gbuffer, lightmapIndirect);
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

		// "Permutate" output ids.
		Guid idGenerated = configuration->getSeedGuid().permutate(0);
		Guid idLightMap = configuration->getSeedGuid().permutate(i * 10 + 1);
		Guid idMesh = configuration->getSeedGuid().permutate(i * 10 + 2);

		// Create a texture build step.
		Ref< render::TextureOutput > textureOutput = new render::TextureOutput();
		textureOutput->m_textureFormat = render::TfR16G16B16A16F;
		textureOutput->m_keepZeroAlpha = false;
		textureOutput->m_hasAlpha = false;
		textureOutput->m_ignoreAlpha = true;
		textureOutput->m_linearGamma = true;
		textureOutput->m_enableCompression = false;
		textureOutput->m_sharpenRadius = 0;
		textureOutput->m_systemTexture = true;
		textureOutput->m_generateMips = false;

		pipelineBuilder->buildOutput(
			textureOutput,
			L"Generated/" + idGenerated.format() + L"/" + idLightMap.format() + L"/__Texture__",
			idLightMap,
			lightmap
		);

		// Modify model materials to use our illumination texture.
		AlignedVector< model::Material > materials = model->getMaterials();
		for (auto& material : materials)
		{
			material.setBlendOperator(model::Material::BoDecal);
			material.setLightMap(model::Material::Map(L"__Illumination__", channel, false), 1.0f);
		}
		model->setMaterials(materials);

		// Create a new mesh asset which use the fresh baked illumination texture.
		auto materialTextures = meshAssets[i]->getMaterialTextures();
		materialTextures[L"__Illumination__"] = idLightMap;

		Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
		outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
		outputMeshAsset->setMaterialTextures(materialTextures);
		pipelineBuilder->buildOutput(
			outputMeshAsset,
			L"Generated/" + idGenerated.format() + L"/" + idMesh.format() + L"/__Mesh__",
			idMesh,
			model
		);

		 // Replace mesh reference to our synthesized mesh instead.
		meshEntityData->getComponent< mesh::MeshComponentData >()->setMesh(resource::Id< mesh::IMesh >(
			idMesh
		));

		double TendWrite = timer.getElapsedTime();
		timer.start();

		log::info << L"Lightmap time breakdown;" << Endl;
		log::info << L"  gbuffer    " << int32_t(TendGBuffer * 1000.0) << L" ms." << Endl;
		log::info << L"  preprocess " << int32_t(TendPreProcess * 1000.0) << L" ms." << Endl;
		log::info << L"  trace      " << int32_t(TendTrace * 1000.0) << L" ms." << Endl;
		log::info << L"  filter     " << int32_t((TendFilter) * 1000.0) << L" ms." << Endl;
		log::info << L"  output     " << int32_t((TendWrite) * 1000.0) << L" ms." << Endl;
	}

	return true;
}

	}
}