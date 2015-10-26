#include <cstring>
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Math/SahTree.h"
#include "Core/Math/Winding3.h"
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
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Illuminate/Editor/GBuffer.h"
#include "Illuminate/Editor/IlluminateEntityData.h"
#include "Illuminate/Editor/IlluminateEntityPipeline.h"
#include "Illuminate/Editor/JobTraceDirect.h"
#include "Illuminate/Editor/JobTraceIndirect.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/UnwrapUV.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "World/Entity/DirectionalLightEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/PointLightEntityData.h"

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

const int32_t c_outputWidth = 2048;
const int32_t c_outputHeight = 2048;
const int32_t c_jobTileWidth = 128;
const int32_t c_jobTileHeight = 128;

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
	RefArray< world::DirectionalLightEntityData >& outDirectionalLightEntityData,
	RefArray< world::PointLightEntityData >& outPointLightEntityData,
	RefArray< mesh::MeshEntityData >& outMeshEntityData
)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (mesh::MeshEntityData* meshEntityData = dynamic_type_cast< mesh::MeshEntityData* >(objectMember->get()))
			outMeshEntityData.push_back(meshEntityData);
		else if (world::DirectionalLightEntityData* directionalLightEntityData = dynamic_type_cast< world::DirectionalLightEntityData* >(objectMember->get()))
			outDirectionalLightEntityData.push_back(directionalLightEntityData);
		else if (world::PointLightEntityData* pointLightEntityData = dynamic_type_cast< world::PointLightEntityData* >(objectMember->get()))
			outPointLightEntityData.push_back(pointLightEntityData);
		else if (objectMember->get())
			collectTraceEntities(objectMember->get(), outDirectionalLightEntityData, outPointLightEntityData, outMeshEntityData);
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.illuminate.IlluminateEntityPipeline", 0, IlluminateEntityPipeline, world::EntityPipeline)

IlluminateEntityPipeline::IlluminateEntityPipeline()
:	m_targetEditor(false)
{
}

bool IlluminateEntityPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
	m_targetEditor = settings->getProperty< PropertyBoolean >(L"Pipeline.TargetEditor", false);
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
	if (!m_targetEditor)
	{
		const IlluminateEntityData* sourceIlluminateEntityData = checked_type_cast< const IlluminateEntityData* >(sourceAsset);

		// Flatten entire hierarchy of illuminate entity.
		Ref< IlluminateEntityData > illumEntityData = checked_type_cast< IlluminateEntityData* >(resolveAllExternal(pipelineDepends, sourceIlluminateEntityData));
		if (!illumEntityData)
		{
			log::error << L"IlluminateEntityPipeline failed; unable to resolve all external entities" << Endl;
			return 0;
		}

		// Get all trace entities.
		RefArray< world::DirectionalLightEntityData > directionalLightEntityData;
		RefArray< world::PointLightEntityData > pointLightEntityData;
		RefArray< mesh::MeshEntityData > meshEntityData;
		collectTraceEntities(illumEntityData, directionalLightEntityData, pointLightEntityData, meshEntityData);

		// Add dependencies to all mesh assets.
		for (RefArray< mesh::MeshEntityData >::const_iterator i = meshEntityData.begin(); i != meshEntityData.end(); ++i)
			pipelineDepends->addDependency((*i)->getMesh(), editor::PdfUse);
	}

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
	if (!m_targetEditor)
	{
		const IlluminateEntityData* sourceIlluminateEntityData = checked_type_cast< const IlluminateEntityData* >(sourceAsset);

		// Flatten entire hierarchy of illuminate entity.
		Ref< IlluminateEntityData > illumEntityData = checked_type_cast< IlluminateEntityData* >(resolveAllExternal(pipelineBuilder, sourceIlluminateEntityData));
		if (!illumEntityData)
		{
			log::error << L"IlluminateEntityPipeline failed; unable to resolve all external entities" << Endl;
			return 0;
		}

		// Get all trace entities.
		RefArray< world::DirectionalLightEntityData > directionalLightEntityData;
		RefArray< world::PointLightEntityData > pointLightEntityData;
		RefArray< mesh::MeshEntityData > meshEntityData;
		collectTraceEntities(illumEntityData, directionalLightEntityData, pointLightEntityData, meshEntityData);

		// Setup lights.
		AlignedVector< Light > lights;

		for (RefArray< world::DirectionalLightEntityData >::const_iterator i = directionalLightEntityData.begin(); i != directionalLightEntityData.end(); ++i)
		{
			Light light;
			light.type = 0;
			light.position = Vector4::origo();
			light.direction = (*i)->getTransform().rotation() * Vector4(0.0f, -1.0f, 0.0f);
			light.sunColor = Color4f((*i)->getSunColor());
			light.baseColor = Color4f((*i)->getBaseColor());
			light.shadowColor = Color4f((*i)->getShadowColor());
			light.range = Scalar(0.0f);
			lights.push_back(light);
		}

		for (RefArray< world::PointLightEntityData >::const_iterator i = pointLightEntityData.begin(); i != pointLightEntityData.end(); ++i)
		{
			Light light;
			light.type = 1;
			light.position = (*i)->getTransform().translation().xyz1();
			light.direction = Vector4::zero();
			light.sunColor = Color4f((*i)->getSunColor());
			light.range = Scalar((*i)->getRange());
			lights.push_back(light);
		}

		Ref< model::Model > mergedModel = new model::Model();
		std::map< std::wstring, Guid > meshMaterialTextures;

		// Merge all models into one, big, model.
		log::info << L"Merging meshes..." << Endl;
		for (RefArray< mesh::MeshEntityData >::const_iterator i = meshEntityData.begin(); i != meshEntityData.end(); ++i)
		{
			Ref< mesh::MeshAsset > meshAsset = pipelineBuilder->getSourceDatabase()->getObjectReadOnly< mesh::MeshAsset >((*i)->getMesh());
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

			if (!model::MergeModel(*model, (*i)->getTransform(), 0.01f).apply(*mergedModel))
				return false;

			meshMaterialTextures.insert(
				meshAsset->getMaterialTextures().begin(),
				meshAsset->getMaterialTextures().end()
			);
		}

		model::CleanDegenerate().apply(*mergedModel);

		// Get next free channel to store lightmap UV.
		uint32_t channel = mergedModel->getAvailableTexCoordChannel();

		// UV unwrap entire model.
		log::info << L"UV unwrapping, using channel " << channel << L"..." << Endl;
		if (!model::UnwrapUV(channel, 3.0f, 1.0f / c_outputWidth, 1.0f / c_outputHeight).apply(*mergedModel))
		{
			log::error << L"IlluminateEntityPipeline failed; unable to unwrap UV." << Endl;
			return 0;
		}

		// Setup tracer.
		log::info << L"Preparing tracer..." << Endl;
		const std::vector< model::Polygon >& polygons = mergedModel->getPolygons();
		std::vector< model::Vertex > vertices = mergedModel->getVertices();

		// Create 3d windings.
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
		gbuffer.create(surfaces, c_outputWidth, c_outputHeight);

		// Create output image.
		Ref< drawing::Image > outputImageRadiance = new drawing::Image(drawing::PixelFormat::getRGBAF32(), c_outputWidth, c_outputHeight);
		outputImageRadiance->clear(Color4f(0.0f, 0.0f, 1.0f, 0.0f));

		RefArray< Job > jobs;

		log::info << L"Tracing direct lighting..." << Endl;
		std::list< JobTraceDirect* > tracesDirect;
		for (int32_t y = 0; y < c_outputHeight; y += c_jobTileHeight)
		{
			for (int32_t x = 0; x < c_outputWidth; x += c_jobTileWidth)
			{
				JobTraceDirect* trace = new JobTraceDirect(
					x,
					y,
					sah,
					gbuffer,
					lights,
					outputImageRadiance,
					sourceIlluminateEntityData->getShadowSamples(),
					sourceIlluminateEntityData->getPointLightRadius()
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

		log::info << L"Dilating direct light map..." << Endl;
		drawing::DilateFilter dilateFilter(4);
		outputImageRadiance->apply(&dilateFilter);

		if (illumEntityData->getDirectConvolveRadius() > 0)
		{
			log::info << L"Convolving direct lighting..." << Endl;
			outputImageRadiance->apply(drawing::ConvolutionFilter::createGaussianBlur(illumEntityData->getDirectConvolveRadius()));
		}

		if (illumEntityData->traceIndirectLighting())
		{
			log::info << L"Tracing indirect lighting..." << Endl;
			std::list< JobTraceIndirect* > tracesIndirect;

			Ref< drawing::Image > outputImageIndirect[] = 
			{
				new drawing::Image(drawing::PixelFormat::getRGBAF32(), c_outputWidth, c_outputHeight),
				new drawing::Image(drawing::PixelFormat::getRGBAF32(), c_outputWidth, c_outputHeight)
			};

			Ref< drawing::Image > imageIrradiance = outputImageRadiance;

			for (int32_t i = 0; i < illumEntityData->getIndirectTraceIterations(); ++i)
			{
				Ref< drawing::Image > outputImageIndirectTarget = outputImageIndirect[i % 2];
				outputImageIndirectTarget->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

				for (int32_t y = 0; y < c_outputHeight; y += c_jobTileHeight)
				{
					for (int32_t x = 0; x < c_outputWidth; x += c_jobTileWidth)
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

				log::info << L"Dilating indirect light map..." << Endl;
				drawing::DilateFilter dilateFilter(4);
				outputImageIndirectTarget->apply(&dilateFilter);

				if (illumEntityData->getIndirectConvolveRadius() > 0)
				{
					log::info << L"Convolving indirect lighting..." << Endl;
					outputImageIndirectTarget->apply(drawing::ConvolutionFilter::createGaussianBlur(illumEntityData->getIndirectConvolveRadius()));
				}

				log::info << L"Merging direct and indirect lighting..." << Endl;
				for (int32_t y = 0; y < c_outputHeight; ++y)
				{
					for (int32_t x = 0; x < c_outputWidth; ++x)
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
			j->setLightMap(model::Material::Map(L"__Illumination__", channel, false));
			j->setEmissive(0.0f);
		}
		mergedModel->setMaterials(materials);

		// Create a new mesh asset which use the fresh baked illumination texture.
		meshMaterialTextures[L"__Illumination__"] = illumEntityData->getSeedGuid().permutate(0);

		Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
		outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
		outputMeshAsset->setMaterialTextures(meshMaterialTextures);
		outputMeshAsset->setGenerateOccluder(false);

		pipelineBuilder->buildOutput(
			outputMeshAsset,
			L"Generated/__Illumination__Mesh__" + illumEntityData->getSeedGuid().permutate(1).format(),
			illumEntityData->getSeedGuid().permutate(1),
			mergedModel
		);

		// Create new mesh entity.
		Ref< mesh::MeshEntityData > outputMeshEntityData = new mesh::MeshEntityData();
		outputMeshEntityData->setName(L"__Illumination__");
		outputMeshEntityData->setMesh(resource::Id< mesh::IMesh >(illumEntityData->getSeedGuid().permutate(1)));

		// Replace entire illuminate entity group with fresh baked mesh entity.
		return outputMeshEntityData;
	}
	else
		return world::EntityPipeline::buildOutput(
			pipelineBuilder,
			sourceAsset
		);
}

	}
}
