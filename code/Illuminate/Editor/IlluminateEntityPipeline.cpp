/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Core/Functor/Functor.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Math/SahTree.h"
#include "Core/Math/Winding3.h"
#include "Core/Misc/String.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Filters/DilateFilter.h"
#include "Editor/Asset.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Illuminate/Editor/CubeProbe.h"
#include "Illuminate/Editor/GBuffer.h"
#include "Illuminate/Editor/IlluminateEntityData.h"
#include "Illuminate/Editor/IlluminateEntityPipeline.h"
#include "Illuminate/Editor/JobTraceDirect.h"
#include "Illuminate/Editor/JobTraceIndirect.h"
#include "Illuminate/Editor/JobTraceOcclusion.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/UnwrapUV.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/LightComponentData.h"

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

const int32_t c_jobTileSize = 128;

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
			{
				log::error << L"IlluminateEntityPipeline failed; Unable to read external entity." << Endl;
				return 0;
			}

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveAllExternal(pipeline, externalEntityData));
			if (!resolvedEntityData)
			{
				log::error << L"IlluminateEntityPipeline failed; Unable to resolve external entity." << Endl;
				return 0;
			}

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
			if (componentEntityData->getComponent< world::LightComponentData >() != 0)
				outLightEntityData.push_back(componentEntityData);
			if (componentEntityData->getComponent< mesh::MeshComponentData >() != 0)
				outMeshEntityData.push_back(componentEntityData);
		}
		else if (objectMember->get())
			collectTraceEntities(objectMember->get(), outLightEntityData, outMeshEntityData);
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.illuminate.IlluminateEntityPipeline", 0, IlluminateEntityPipeline, world::EntityPipeline)

IlluminateEntityPipeline::IlluminateEntityPipeline()
:	m_build(true)
{
}

bool IlluminateEntityPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	m_build = settings->getProperty< bool >(L"IlluminatePipeline.Build", true);
	return true;
}

TypeInfoSet IlluminateEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IlluminateEntityData >());
	return typeSet;
}

bool IlluminateEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	return world::EntityPipeline::buildDependencies(
		pipelineDepends,
		sourceInstance,
		sourceAsset,
		outputPath,
		outputGuid
	);
}

Ref< ISerializable > IlluminateEntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	RefArray< Job > jobs;
	if (m_build)
	{
		const IlluminateEntityData* sourceIlluminateEntityData = checked_type_cast< const IlluminateEntityData* >(sourceAsset);

		// Flatten entire hierarchy of illuminate entity.
		Ref< IlluminateEntityData > illumEntityData = checked_type_cast< IlluminateEntityData* >(resolveAllExternal(pipelineBuilder, sourceIlluminateEntityData));
		if (!illumEntityData)
		{
			log::error << L"IlluminateEntityPipeline failed; unable to resolve all external entities" << Endl;
			return 0;
		}

		int32_t margin = 1;
		margin = std::max(margin, illumEntityData->getDirectConvolveRadius() + 1);
		margin = std::max(margin, illumEntityData->getIndirectConvolveRadius() + 1);

		// Get all trace entities.
		RefArray< world::ComponentEntityData > lightEntityData;
		RefArray< world::ComponentEntityData > meshEntityData;
		collectTraceEntities(illumEntityData, lightEntityData, meshEntityData);

		// Setup lights.
		AlignedVector< Light > lights;

		for (RefArray< world::ComponentEntityData >::const_iterator i = lightEntityData.begin(); i != lightEntityData.end(); ++i)
		{
			world::LightComponentData* lightComponentData = (*i)->getComponent< world::LightComponentData >();
			T_FATAL_ASSERT (lightComponentData != 0);

			Light light;
			if (lightComponentData->getLightType() == world::LtDirectional)
			{
				light.type = 0;
				light.position = Vector4::origo();
				light.direction = (*i)->getTransform().rotation() * Vector4(0.0f, -1.0f, 0.0f);
				light.sunColor = Color4f(lightComponentData->getSunColor());
				light.baseColor = Color4f(lightComponentData->getBaseColor());
				light.shadowColor = Color4f(lightComponentData->getShadowColor());
				light.range = Scalar(0.0f);
				lights.push_back(light);
			}
			else if (lightComponentData->getLightType() == world::LtPoint)
			{
				light.type = 1;
				light.position = (*i)->getTransform().translation().xyz1();
				light.direction = Vector4::zero();
				light.sunColor = Color4f(lightComponentData->getSunColor());
				light.range = Scalar(lightComponentData->getRange());
				lights.push_back(light);
			}
			else if (lightComponentData->getLightType() == world::LtProbe)
			{
				Ref< const editor::Asset > probeAsset = pipelineBuilder->getObjectReadOnly< editor::Asset >(lightComponentData->getProbeDiffuseTexture());
				if (!probeAsset)
					return 0;

				Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), probeAsset->getFileName().getOriginal());
				if (!file)
				{
					log::error << L"IlluminateEntityPipeline failed; unable to open source image \"" << probeAsset->getFileName().getOriginal() << L"\"" << Endl;
					return false;
				}

				Ref< drawing::Image > image = drawing::Image::load(file, probeAsset->getFileName().getExtension());
				if (!image)
				{
					log::error << L"IlluminateEntityPipeline failed; unable to load source image \"" << probeAsset->getFileName().getOriginal() << L"\"" << Endl;
					return false;
				}

				file->close();

				light.type = 2;
				light.probe = new CubeProbe(image);
				lights.push_back(light);
			}
		}

		Ref< model::Model > mergedModel = new model::Model();
		std::map< std::wstring, Guid > meshMaterialTextures;

		// Merge all models into one, big, model.
		log::info << L"Merging meshes..." << Endl;
		for (RefArray< world::ComponentEntityData >::const_iterator i = meshEntityData.begin(); i != meshEntityData.end(); ++i)
		{
			Ref< mesh::MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< mesh::MeshAsset >((*i)->getComponent< mesh::MeshComponentData >()->getMesh());
			if (!meshAsset)
				continue;

			Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), meshAsset->getFileName().getOriginal());
			if (!file)
			{
				log::warning << L"Unable to open file \"" << meshAsset->getFileName().getOriginal() << L"\"" << Endl;
				continue;
			}

			Ref< model::Model > model = model::ModelFormat::readAny(
				file,
				meshAsset->getFileName().getExtension()
			);
			if (!model)
			{
				log::warning << L"Unable to read model \"" << meshAsset->getFileName().getOriginal() << L"\"" << Endl;
				continue;
			}

			model->clear(model::Model::CfColors | model::Model::CfJoints);

			if (!model::MergeModel(*model, (*i)->getTransform(), 0.01f).apply(*mergedModel))
				return 0;

			meshMaterialTextures.insert(
				meshAsset->getMaterialTextures().begin(),
				meshAsset->getMaterialTextures().end()
			);
		}

		model::CleanDegenerate().apply(*mergedModel);
		model::CleanDuplicates(0.01f).apply(*mergedModel);

		// Create 3d windings.
		const std::vector< model::Polygon >& polygons = mergedModel->getPolygons();
		std::vector< model::Vertex > vertices = mergedModel->getVertices();
		AlignedVector< Winding3 > windings(polygons.size());
		for (uint32_t j = 0; j < polygons.size(); ++j)
		{
			Winding3& w = windings[j];
				
			const std::vector< uint32_t >& vertexIndices = polygons[j].getVertices();
			if (vertexIndices.size() < 3 || vertexIndices.size() > 16)
				continue;

			for (std::vector< uint32_t >::const_iterator k = vertexIndices.begin(); k != vertexIndices.end(); ++k)
			{
				const model::Vertex& polyVertex = mergedModel->getVertex(*k);
				const Vector4& polyVertexPosition = mergedModel->getPosition(polyVertex.getPosition());
				w.push(polyVertexPosition);
			}
		}

		// Get next free channel to store lightmap UV.
		uint32_t channel = mergedModel->getAvailableTexCoordChannel();
		log::info << L"UV unwrapping, using channel " << channel << L"..." << Endl;

		// Calculate output size from lumel density.
		float totalWorldArea = 0.0f;
		for (AlignedVector< Winding3 >::const_iterator i = windings.begin(); i != windings.end(); ++i)
			totalWorldArea += std::abs(i->area());

		float totalLightMapArea = sourceIlluminateEntityData->getLumelDensity() * sourceIlluminateEntityData->getLumelDensity() * totalWorldArea;
		float size = std::sqrt(totalLightMapArea);

		int32_t initialOutputSize = alignUp(int32_t(size + 0.5f), c_jobTileSize);
		log::info << L"Lumel density " << sourceIlluminateEntityData->getLumelDensity() << L" lumels/unit => initial lightmap size " << initialOutputSize << Endl;

		// UV unwrap entire model.
		int32_t outputSize = initialOutputSize;
		if (!model::UnwrapUV(channel, sourceIlluminateEntityData->getLumelDensity(), outputSize, margin).apply(*mergedModel))
		{
			outputSize *= 2;
			log::info << L"First UV unwrapping attempt failed, enlarging lightmap to " << outputSize << Endl;
			if (!model::UnwrapUV(channel, sourceIlluminateEntityData->getLumelDensity(), outputSize, margin).apply(*mergedModel))
			{
				log::error << L"IlluminateEntityPipeline failed; unable to unwrap UV." << Endl;
				return 0;
			}
		}

		model::ModelFormat::writeAny(Path(L"./MergedModel.obj"), mergedModel);

		// Setup tracer.
		log::info << L"Preparing tracer..." << Endl;

		// Create traceable surfaces.
		AlignedVector< Surface > surfaces(polygons.size());
		for (uint32_t j = 0; j < polygons.size(); ++j)
		{
			Surface& s = surfaces[j];

			const std::vector< uint32_t >& vertexIndices = polygons[j].getVertices();
			if (vertexIndices.size() < 3 || vertexIndices.size() > 16)
				continue;

			s.count = int32_t(vertexIndices.size());
			for (int32_t k = 0; k < s.count; ++k)
			{
				const model::Vertex& polyVertex = mergedModel->getVertex(vertexIndices[k]);

				s.points[k] = mergedModel->getPosition(polyVertex.getPosition());
				s.texCoords[k] = mergedModel->getTexCoord(polyVertex.getTexCoord(channel));
				s.normals[k] = mergedModel->getNormal(polyVertex.getNormal());
			}

			Plane plane;
			if (windings[j].getPlane(plane))
				s.normal = plane.normal();
			else
				s.normal = Vector4::zero();

			uint32_t materialId = polygons[j].getMaterial();
			const model::Material& material = mergedModel->getMaterial(materialId);

			float rgba[4];
			material.getColor().getRGBA32F(rgba);

			s.color = Color4f(rgba);
			s.emissive = Scalar(material.getEmissive());
		}

		// Build acceleration tree.
		SahTree sah;
		sah.build(windings);

		// Create GBuffer.
		GBuffer gbuffer;
		gbuffer.create(surfaces, outputSize, outputSize);
		gbuffer.saveAsImages(Path(L"."));
		gbuffer.dilate(margin);

		// Create images.
		Ref< drawing::Image > outputImageRadiance = new drawing::Image(drawing::PixelFormat::getRGBAF32(), outputSize, outputSize);
		outputImageRadiance->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

		Ref< drawing::Image > outputImageOcclusion = new drawing::Image(drawing::PixelFormat::getR32F(), outputSize, outputSize);
		outputImageOcclusion->clear(Color4f(1.0f, 1.0f, 1.0f, 0.0f));

		if (sourceIlluminateEntityData->traceOcclusion())
		{
			log::info << L"Tracing occlusion..." << Endl;
			std::list< JobTraceOcclusion* > tracesOcclusion;
			for (int32_t y = 0; y < outputSize; y += c_jobTileSize)
			{
				for (int32_t x = 0; x < outputSize; x += c_jobTileSize)
				{
					JobTraceOcclusion* trace = new JobTraceOcclusion(
						x,
						y,
						sah,
						gbuffer,
						outputImageOcclusion,
						64
					);

					Ref< Job > job = JobManager::getInstance().add(makeFunctor< JobTraceOcclusion >(trace, &JobTraceOcclusion::execute));
					if (!job)
						return 0;

					tracesOcclusion.push_back(trace);
					jobs.push_back(job);
				}
			}

			for (RefArray< Job >::iterator j = jobs.begin(); j != jobs.end(); ++j)
				(*j)->wait();

			for(std::list< JobTraceOcclusion* >::iterator j = tracesOcclusion.begin(); j != tracesOcclusion.end(); ++j)
				delete *j;

			tracesOcclusion.clear();
			jobs.clear();

			if (illumEntityData->getDirectConvolveRadius() > 0)
			{
				log::info << L"Convolving occlusion..." << Endl;
				drawing::DilateFilter dilateFilter(illumEntityData->getDirectConvolveRadius());
				outputImageOcclusion->apply(&dilateFilter);
				outputImageOcclusion->apply(drawing::ConvolutionFilter::createGaussianBlur(illumEntityData->getDirectConvolveRadius()));
			}

			outputImageOcclusion->save(L"Occlusion.png");
		}

		log::info << L"Tracing direct lighting..." << Endl;
		std::list< JobTraceDirect* > tracesDirect;
		for (int32_t y = 0; y < outputSize; y += c_jobTileSize)
		{
			for (int32_t x = 0; x < outputSize; x += c_jobTileSize)
			{
				JobTraceDirect* trace = new JobTraceDirect(
					x,
					y,
					sah,
					gbuffer,
					lights,
					outputImageRadiance,
					outputImageOcclusion,
					sourceIlluminateEntityData->getPointLightRadius(),
					sourceIlluminateEntityData->getShadowSamples(),
					sourceIlluminateEntityData->getProbeSamples(),
					sourceIlluminateEntityData->getProbeCoeff(),
					sourceIlluminateEntityData->getProbeSpread(),
					sourceIlluminateEntityData->getProbeShadowSpread()
				);

				Ref< Job > job = JobManager::getInstance().add(makeFunctor< JobTraceDirect >(trace, &JobTraceDirect::execute));
				if (!job)
					return 0;

				tracesDirect.push_back(trace);
				jobs.push_back(job);
			}
		}

		for (RefArray< Job >::iterator j = jobs.begin(); j != jobs.end(); ++j)
			(*j)->wait();

		for(std::list< JobTraceDirect* >::iterator j = tracesDirect.begin(); j != tracesDirect.end(); ++j)
			delete *j;

		tracesDirect.clear();
		jobs.clear();

		if (illumEntityData->getDirectConvolveRadius() > 0)
		{
			log::info << L"Convolving direct lighting..." << Endl;
			drawing::DilateFilter dilateFilter(illumEntityData->getDirectConvolveRadius());
			outputImageRadiance->apply(&dilateFilter);
			outputImageRadiance->apply(drawing::ConvolutionFilter::createGaussianBlur(illumEntityData->getDirectConvolveRadius()));
		}

		outputImageRadiance->save(L"Direct.png");

		if (illumEntityData->traceIndirectLighting())
		{
			log::info << L"Tracing indirect lighting..." << Endl;
			std::list< JobTraceIndirect* > tracesIndirect;

			Ref< drawing::Image > outputImageIndirect[] = 
			{
				new drawing::Image(drawing::PixelFormat::getRGBAF32(), outputSize, outputSize),
				new drawing::Image(drawing::PixelFormat::getRGBAF32(), outputSize, outputSize)
			};

			Ref< drawing::Image > imageIrradiance = outputImageRadiance;

			for (int32_t i = 0; i < illumEntityData->getIndirectTraceIterations(); ++i)
			{
				Ref< drawing::Image > outputImageIndirectTarget = outputImageIndirect[i % 2];
				outputImageIndirectTarget->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

				for (int32_t y = 0; y < outputSize; y += c_jobTileSize)
				{
					for (int32_t x = 0; x < outputSize; x += c_jobTileSize)
					{
						JobTraceIndirect* trace = new JobTraceIndirect(
							x,
							y,
							sah,
							gbuffer,
							surfaces,
							imageIrradiance,
							outputImageIndirectTarget,
							illumEntityData->getIndirectTraceSamples()
						);

						Ref< Job > job = JobManager::getInstance().add(makeFunctor< JobTraceIndirect >(trace, &JobTraceIndirect::execute));
						if (!job)
							return 0;

						tracesIndirect.push_back(trace);
						jobs.push_back(job);
					}
				}

				for (RefArray< Job >::iterator j = jobs.begin(); j != jobs.end(); ++j)
					(*j)->wait();

				for(std::list< JobTraceIndirect* >::iterator j = tracesIndirect.begin(); j != tracesIndirect.end(); ++j)
					delete *j;

				tracesIndirect.clear();
				jobs.clear();

				if (illumEntityData->getIndirectConvolveRadius() > 0)
				{
					log::info << L"Convolving indirect lighting..." << Endl;
					drawing::DilateFilter dilateFilter(illumEntityData->getIndirectConvolveRadius());
					outputImageIndirectTarget->apply(&dilateFilter);
					outputImageIndirectTarget->apply(drawing::ConvolutionFilter::createGaussianBlur(illumEntityData->getIndirectConvolveRadius()));
				}

				outputImageIndirectTarget->save(L"Indirect_" + toString(i) + L".png");

				log::info << L"Merging direct and indirect lighting..." << Endl;
				for (int32_t y = 0; y < outputSize; ++y)
				{
					for (int32_t x = 0; x < outputSize; ++x)
					{
						Color4f inA, inB;
						outputImageIndirectTarget->getPixelUnsafe(x, y, inA);
						imageIrradiance->getPixelUnsafe(x, y, inB);
						outputImageRadiance->setPixelUnsafe(x, y, (inA + inB) * Color4f(1.0f, 1.0f, 1.0f, 0.0f) + Color4f(0.0f, 0.0f, 0.0f, 1.0f));
					}
				}

				imageIrradiance = outputImageIndirectTarget;
			}
		}

#if defined(_DEBUG)
		// Save light-map for debugging.
		outputImageRadiance->save(illumEntityData->getSeedGuid().format() + L".png");
		model::ModelFormat::writeAny(illumEntityData->getSeedGuid().format() + L".obj", mergedModel);
#endif

		log::info << L"Creating resources..." << Endl;

		// Create a texture build step.
		Ref< render::TextureOutput > textureOutput = new render::TextureOutput();
		if (illumEntityData->highDynamicRange() && !illumEntityData->compressLightMap())
			textureOutput->m_textureFormat = render::TfR16G16B16A16F;
		textureOutput->m_keepZeroAlpha = false;
		textureOutput->m_hasAlpha = false;
		textureOutput->m_ignoreAlpha = true;
		textureOutput->m_linearGamma = true;
		textureOutput->m_enableCompression = illumEntityData->compressLightMap();
		textureOutput->m_sharpenRadius = 0;
		textureOutput->m_systemTexture = true;
		textureOutput->m_generateMips = false;

		// Measure max range of illumination texture, also encode lightmap with RGBM encoding.
		float lumelRange = 1.0f;
		if (!illumEntityData->highDynamicRange())
		{
			for (int32_t y = 0; y < outputSize; ++y)
			{
				for (int32_t x = 0; x < outputSize; ++x)
				{
					Color4f lumel;
					outputImageRadiance->getPixelUnsafe(x, y, lumel);

					lumel = lumel / Scalar(6.0f);

					Scalar mx = max(
						max(lumel.getRed(), lumel.getGreen()),
						max(lumel.getBlue(), Scalar(1e-6f))
					);

					mx = clamp(mx, Scalar(0.0f), Scalar(1.0f));
					mx = Scalar(std::ceil(mx * 255.0f)) / Scalar(255.0f);

					lumel = lumel / mx;
					lumel.setAlpha(mx);

					outputImageRadiance->setPixelUnsafe(x, y, lumel);
				}
			}

			textureOutput->m_hasAlpha = true;
			textureOutput->m_ignoreAlpha = false;
		}

		pipelineBuilder->buildOutput(
			textureOutput,
			L"Generated/__Illumination__Texture__" + illumEntityData->getSeedGuid().permutate(0).format(),
			illumEntityData->getSeedGuid().permutate(0),
			outputImageRadiance
		);

		// Modify model materials to use our illumination texture.
		std::vector< model::Material > materials = mergedModel->getMaterials();
		for (std::vector< model::Material >::iterator j = materials.begin(); j != materials.end(); ++j)
		{
			j->setLightMap(model::Material::Map(L"__Illumination__", channel, false), lumelRange);
			j->setEmissive(0.0f);
		}
		mergedModel->setMaterials(materials);

		// Create a new mesh asset which use the fresh baked illumination texture.
		meshMaterialTextures[L"__Illumination__"] = illumEntityData->getSeedGuid().permutate(0);

		Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
		outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
		outputMeshAsset->setMaterialTextures(meshMaterialTextures);

		pipelineBuilder->buildOutput(
			outputMeshAsset,
			L"Generated/__Illumination__Mesh__" + illumEntityData->getSeedGuid().permutate(1).format(),
			illumEntityData->getSeedGuid().permutate(1),
			mergedModel
		);

		// Create new mesh entity.
		Ref< world::ComponentEntityData > outputEntityData = new world::ComponentEntityData();
		outputEntityData->setName(L"__Illumination__");
		outputEntityData->setComponent(new mesh::MeshComponentData(
			resource::Id< mesh::IMesh >(illumEntityData->getSeedGuid().permutate(1))
		));

		// Replace entire illuminate entity group with fresh baked mesh entity.
		return outputEntityData;
	}
	else
		return world::EntityPipeline::buildOutput(
			pipelineBuilder,
			sourceAsset
		);
}

	}
}
