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
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/LightComponentData.h"

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

// class WrappedSHFunction : public render::SHFunction
// {
// public:
// 	WrappedSHFunction(const RayTracer& tracer, RayTracer::Context* tracerContext, const Vector4& origin)
// 	:	m_tracer(tracer)
// 	,	m_tracerContext(tracerContext)
// 	,	m_origin(origin)
// 	{
// 	}

// 	virtual Vector4 evaluate(float phi, float theta, const Vector4& unit) const override final
// 	{
// 		return m_tracer.traceIndirect(m_tracerContext, m_origin, unit);
// 	}

// private:
// 	const RayTracer& m_tracer;
// 	Ref< RayTracer::Context > m_tracerContext;
// 	Vector4 m_origin;
// };

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
	RefArray< world::ComponentEntityData >& outMeshEntityData
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
		}
		else if (objectMember->get())
			collectTraceEntities(objectMember->get(), outLightEntityData, outMeshEntityData);
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.illuminate.IlluminatePipelineOperator", 0, IlluminatePipelineOperator, scene::IScenePipelineOperator)

bool IlluminatePipelineOperator::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
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
	Ref< IRayTracer > rayTracer = new RayTracerEmbree(); // RayTracerLocal();
	if (!rayTracer->create(configuration))
		return false;

	RefArray< world::ComponentEntityData > lightEntityDatas;
	RefArray< world::ComponentEntityData > meshEntityDatas;

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
			collectTraceEntities(flattenedLayer, lightEntityDatas, meshEntityDatas);

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

	// // Raytrace IBL probes.
	// render::SHEngine shEngine(3);
	// shEngine.generateSamplePoints(20000);

	// RefArray< Job > jobs;
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

	// 	auto job = JobManager::getInstance().add(makeFunctor([&, lightComponentData]() {
	// 		Ref< render::SHCoeffs > shCoeffs = new render::SHCoeffs();

	// 		Ref< RayTracer::Context > context = tracer.createContext();
	// 		WrappedSHFunction shFunction(tracer, context, position);
	// 		shEngine.generateCoefficients(&shFunction, *shCoeffs);

	// 		lightComponentData->setSHCoeffs(shCoeffs);
	// 	}));
	// 	if (!job)
	// 		return false;

	// 	jobs.push_back(job);
	// }
	// while (!jobs.empty())
	// {
	// 	jobs.back()->wait();
	// 	jobs.pop_back();
	// }

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

		int32_t outputSize = nearestLog2(int32_t(size + 0.5f));
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

		// Create G-Buffer of mesh's geometry.
		gbuffer.create(outputSize, outputSize, *model, meshEntityData->getTransform(), channel);
		//gbuffer.saveAsImages(meshEntityData->getName() + L"_" + toString(i) + L"_GBuffer");

		// // Adjust gbuffer positions to help fix some shadowing issues.
		// if (configuration->getEnableShadowFix())
		// {
		// 	for (int32_t y = 0; y < outputSize; ++y)
		// 	{
		// 		for (int32_t x = 0; x < outputSize; ++x)
		// 		{
		// 			auto& elm = gbuffer.get(x, y);
		// 			if (elm.polygon == model::c_InvalidIndex)
		// 				continue;

		// 			Vector4 position = elm.position;
		// 			Vector4 normal = elm.normal;

		// 			Vector4 u, v;
		// 			orthogonalFrame(normal, u, v);

		// 			const Scalar l = elm.delta.length();
		// 			const Vector4 d[] = { u, -u, v, -v };

		// 			for (int32_t i = 0; i < 8; ++i)
		// 			{
		// 				int32_t ii = i % 4;

		// 				RayTracer::Result result;
		// 				if (!tracer.trace(tracerContext, position + normal * Scalar(0.01f), d[ii], l, result))
		// 					continue;

		// 				if (dot3(result.normal, d[ii]) > 0.0f)
		// 					continue;

		// 				position = position + d[ii] * result.distance + result.normal * Scalar(0.01f);
		// 			}
		// 		}
		// 	}
		// }

		Timer timer;
		timer.start();

		Ref< drawing::Image > lightmapDirect;
		if (configuration->traceDirect())
			lightmapDirect = rayTracer->traceDirect(&gbuffer);

		Ref< drawing::Image > lightmapIndirect;
		if (configuration->traceIndirect())
			lightmapIndirect = rayTracer->traceIndirect(&gbuffer);

		double TendTrace = timer.getElapsedTime();

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
		if (lightmapIndirect)
			lightmapIndirect->apply(drawing::ConvolutionFilter::createGaussianBlur(1));

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

		// Discard alpha.
		lightmap->clearAlpha(1.0f);

		double TendFilter = timer.getElapsedTime();

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

		log::info << L"Lightmap time breakdown;" << Endl;
		log::info << L"  trace  " << int32_t(TendTrace * 1000.0) << L" ms." << Endl;
		log::info << L"  filter " << int32_t((TendFilter - TendTrace) * 1000.0) << L" ms." << Endl;
		log::info << L"  output " << int32_t((TendWrite - TendFilter) * 1000.0) << L" ms." << Endl;
	}

	return true;
}

	}
}