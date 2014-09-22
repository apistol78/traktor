#include <ctime>
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Core/Math/Triangle.h"
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
#include "Illuminate/Editor/IlluminateEntityData.h"
#include "Illuminate/Editor/IlluminateEntityPipeline.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/Triangulate.h"
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

const Scalar c_traceRayOffset(0.025f);
const int32_t c_outputWidth = 2048;
const int32_t c_outputHeight = 2048;
const int32_t c_gatherSamples = 64;
const int32_t c_jobTileWidth = 32;
const int32_t c_jobTileHeight = 32;

struct GBuffer
{
	Vector4 position;
	Vector4 normal;
};

struct Surface
{
	Vector4 points[3];
	Vector2 texCoords[3];
	Vector4 normal;
	Color4f color;
};

struct Light
{
	int32_t type;		//<! 0 - directional, 1 - point
	Vector4 position;
	Vector4 direction;
	Color4f color;
	Scalar range;
};

class GBufferVisitor
{
public:
	GBufferVisitor(
		const Vector4 P[3],
		const Vector4 N[3],
		AlignedVector< GBuffer >& outGBuffer
	)
	:	m_outGBuffer(outGBuffer)
	{
		for (int i = 0; i < 3; ++i)
		{
			m_P[i] = P[i];
			m_N[i] = N[i];
		}
	}

	void operator () (int32_t x, int32_t y, float alpha, float beta, float gamma)
	{
		Vector4 position = (m_P[0] * Scalar(alpha) + m_P[1] * Scalar(beta) + m_P[2] * Scalar(gamma)).xyz1();
		Vector4 normal = (m_N[0] * Scalar(alpha) + m_N[1] * Scalar(beta) + m_N[2] * Scalar(gamma)).xyz0();
		m_outGBuffer[x + y * c_outputWidth].position = position;
		m_outGBuffer[x + y * c_outputWidth].normal = normal;
	}

private:
	Vector4 m_P[3];
	Vector4 m_N[3];
	AlignedVector< GBuffer >& m_outGBuffer;
};

void jobTraceLightmap(
	int32_t tileX,
	int32_t tileY,
	const SahTree& sah,
	const AlignedVector< GBuffer >& gbuffer,
	const AlignedVector< Surface >& surfaces,
	const AlignedVector< Light >& lights,
	drawing::Image* outputImageDirect,
	drawing::Image* outputImageIndirect
)
{
	RandomGeometry random(std::clock());
	SahTree::QueryResult result;
	Color4f tmp;

	for (int32_t y = tileY; y < tileY + c_jobTileHeight; ++y)
	{
		for (int32_t x = tileX; x < tileX + c_jobTileWidth; ++x)
		{
			const Vector4& position = gbuffer[x + y * c_outputWidth].position;
			if (position.w() < FUZZY_EPSILON)
				continue;

			const Vector4& normal = gbuffer[x + y * c_outputWidth].normal;

			Vector4 rayOrigin = (position + normal * c_traceRayOffset).xyz1();

			Color4f direct(0.0f, 0.0f, 0.0f, 1.0f);
			Color4f indirect(0.0f, 0.0f, 0.0f, 1.0f);

			// Accumulate direct lighting.
			for (AlignedVector< Light >::const_iterator i = lights.begin(); i != lights.end(); ++i)
			{
				if (i->type == 0)
				{
					Scalar phi = dot3(-i->direction, normal);
					if (phi <= 0.0f)
						continue;

					if (sah.queryAnyIntersection(rayOrigin, -i->direction, 0.0f))
						continue;

					direct += Color4f(phi, phi, phi, 0.0f) * i->color;
				}
				else if (i->type == 1)
				{
					Vector4 lightDirection = (i->position - position).xyz0();
					Scalar lightDistance = lightDirection.normalize();
					if (lightDistance >= i->range)
						continue;

					Scalar phi = dot3(lightDirection, normal);
					if (phi <= 0.0f)
						continue;

					if (sah.queryAnyIntersection(rayOrigin, lightDirection, lightDistance - FUZZY_EPSILON))
						continue;

					Scalar attenuate = Scalar(1.0f) - lightDistance / i->range;
					direct += Color4f(phi, phi, phi, 0.0f) * i->color * attenuate;
				}
			}

			// Gather indirect lighting.
			for (int32_t i = 0; i < c_gatherSamples; )
			{
				Vector4 rayDirection = random.nextUnit();
				Scalar phi = dot3(rayDirection, normal);
				if (phi <= 0.2f)
					continue;
			
				if (sah.queryClosestIntersection(rayOrigin, rayDirection, result))
				{
					const Vector4& P = result.position;
					const Surface& hitSurface = surfaces[result.index];

					Vector4 hitRayOrigin = P + (hitSurface.normal * c_traceRayOffset).xyz1();
					Color4f hitDirect(0.0f, 0.0f, 0.0f, 0.0f);
			
					for (AlignedVector< Light >::const_iterator j = lights.begin(); j != lights.end(); ++j)
					{
						if (j->type == 0)
						{
							Scalar hitPhi = dot3(-j->direction, hitSurface.normal);
							if (hitPhi <= 0.0f)
								continue;

							if (sah.queryAnyIntersection(hitRayOrigin, -j->direction, 0.0f))
								continue;

							hitDirect += Color4f(hitPhi, hitPhi, hitPhi, 0.0f) * j->color;
						}
						else if (j->type == 1)
						{
							Vector4 lightDirection = (j->position - position).xyz0();
							Scalar lightDistance = lightDirection.normalize();
							if (lightDistance >= j->range)
								continue;

							Scalar hitPhi = dot3(lightDirection, hitSurface.normal);
							if (hitPhi <= 0.0f)
								continue;

							if (sah.queryAnyIntersection(hitRayOrigin, lightDirection, lightDistance - FUZZY_EPSILON))
								continue;

							Scalar attenuate = Scalar(1.0f) - lightDistance / j->range;
							hitDirect += Color4f(hitPhi, hitPhi, hitPhi, 0.0f) * j->color * attenuate;
						}
					}
			
					indirect += hitDirect * hitSurface.color;
				}
			
				++i;
			}
			indirect /= Scalar(c_gatherSamples);

			outputImageDirect->setPixel(x, y, direct);
			outputImageIndirect->setPixel(x, y, indirect);
		}
	}
}

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

Guid advanceGuid(const Guid& seed, int32_t iterations)
{
	uint8_t data[16];
	std::memcpy(data, seed, 16);
	*(uint32_t*)data += iterations;
	return Guid(data);
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
	// FIXME
	// Prevent building mesh;es which will be replaced by this entity.

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
	RandomGeometry random;

	if (!m_targetEditor)
	{
		const IlluminateEntityData* sourceIlluminateEntityData = checked_type_cast< const IlluminateEntityData* >(sourceAsset);

		// Flatten entire hierarchy of illuminate entity.
		Ref< IlluminateEntityData > illumEntityData = checked_type_cast< IlluminateEntityData* >(resolveAllExternal(pipelineBuilder, sourceIlluminateEntityData));
		if (!illumEntityData)
		{
			log::error << L"IlluminateEntityPipeline failed; unable to resolve all external entities" << Endl;
			return false;
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
			light.color = Color4f((*i)->getSunColor());
			light.range = Scalar(0.0f);
			lights.push_back(light);
		}

		for (RefArray< world::PointLightEntityData >::const_iterator i = pointLightEntityData.begin(); i != pointLightEntityData.end(); ++i)
		{
			Light light;
			light.type = 1;
			light.position = (*i)->getTransform().translation().xyz1();
			light.direction = Vector4::zero();
			light.color = Color4f((*i)->getSunColor());
			light.range = Scalar((*i)->getRange());
			lights.push_back(light);
		}

		// Bake light onto each mesh entity; need to create unique mesh;es for each entity.
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
		
			if (!model::Triangulate().apply(*model))
				continue;

			// Setup tracer.
			const std::vector< model::Polygon >& polygons = model->getPolygons();
			std::vector< model::Vertex > vertices = model->getVertices();

			AlignedVector< Winding3 > windings(polygons.size());
			for (uint32_t j = 0; j < polygons.size(); ++j)
			{
				Winding3& w = windings[j];
				const std::vector< uint32_t >& vertexIndices = polygons[j].getVertices();
				for (std::vector< uint32_t >::const_iterator k = vertexIndices.begin(); k != vertexIndices.end(); ++k)
				{
					const model::Vertex& polyVertex = model->getVertex(*k);
					const Vector4& polyVertexPosition = model->getPosition(polyVertex.getPosition());
					w.points.push_back(polyVertexPosition);
				}
			}

			AlignedVector< Surface > surfaces(polygons.size());
			for (uint32_t j = 0; j < polygons.size(); ++j)
			{
				Surface& s = surfaces[j];

				const std::vector< uint32_t >& vertexIndices = polygons[j].getVertices();
				for (int32_t k = 0; k < 3; ++k)
				{
					const model::Vertex& polyVertex = model->getVertex(vertexIndices[k]);

					s.points[k] = model->getPosition(polyVertex.getPosition());
					s.texCoords[k] = model->getTexCoord(polyVertex.getTexCoord(0));
				}

				Plane plane;
				if (windings[j].getPlane(plane))
					s.normal = plane.normal();
				else
					s.normal = Vector4::zero();

				uint32_t materialId = polygons[j].getMaterial();
				const model::Material& material = model->getMaterial(materialId);

				float rgba[4];
				material.getColor().getRGBA32F(rgba);

				s.color = Color4f(rgba);
			}

			// Build acceleration tree.
			SahTree sah;
			sah.build(windings);

			// Create GBuffer images.
			const GBuffer zero = { Vector4::zero(), Vector4::zero() };
			AlignedVector< GBuffer > gbuffer(c_outputWidth * c_outputHeight, zero);

			// Trace each polygon in UV space.
			Vector2 dim( c_outputWidth, c_outputHeight );
			Vector2 uv[3];
			Vector4 P[3], N[3];

			// Trace first, direct illumination, pass.
			log::info << L"Generating g-buffer..." << Endl;
			for (uint32_t j = 0; j < polygons.size(); ++j)
			{
				const model::Vertex& v0 = vertices[polygons[j].getVertex(0)];
				const model::Vertex& v1 = vertices[polygons[j].getVertex(1)];
				const model::Vertex& v2 = vertices[polygons[j].getVertex(2)];

				int32_t tc0 = v0.getTexCoord(0);
				int32_t tc1 = v1.getTexCoord(0);
				int32_t tc2 = v2.getTexCoord(0);

				if (tc0 == model::c_InvalidIndex || tc1 == model::c_InvalidIndex || tc2 == model::c_InvalidIndex)
					continue;

				uv[0] = model->getTexCoord(tc0) * dim;
				uv[1] = model->getTexCoord(tc1) * dim;
				uv[2] = model->getTexCoord(tc2) * dim;

				P[0] = model->getPosition(v0.getPosition());
				P[1] = model->getPosition(v1.getPosition());
				P[2] = model->getPosition(v2.getPosition());

				int32_t n0 = v0.getNormal();
				int32_t n1 = v1.getNormal();
				int32_t n2 = v2.getNormal();

				if (n0 == model::c_InvalidIndex || n1 == model::c_InvalidIndex || n2 == model::c_InvalidIndex)
					continue;

				N[0] = model->getNormal(n0);
				N[1] = model->getNormal(n1);
				N[2] = model->getNormal(n2);

				GBufferVisitor visitor1(P, N, gbuffer);
				triangle(uv[0], uv[1], uv[2], visitor1);

				std::swap(P[0], P[2]);
				std::swap(N[0], N[2]);
				std::swap(uv[0], uv[2]);

				GBufferVisitor visitor2(P, N, gbuffer);
				triangle(uv[0], uv[1], uv[2], visitor2);
			}

			// Create output image.
			Ref< drawing::Image > outputImageDirect = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), c_outputWidth, c_outputHeight);
			Ref< drawing::Image > outputImageIndirect = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), c_outputWidth, c_outputHeight);
			
			outputImageDirect->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
			outputImageIndirect->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

			log::info << L"Tracing light-map..." << Endl;

			RefArray< Job > jobs;
			for (int32_t y = 0; y < c_outputHeight; y += c_jobTileHeight)
			{
				for (int32_t x = 0; x < c_outputWidth; x += c_jobTileWidth)
				{
					Ref< Job > job = JobManager::getInstance().add(makeStaticFunctor<
						int32_t,
						int32_t,
						const SahTree&,
						const AlignedVector< GBuffer >&,
						const AlignedVector< Surface >&,
						const AlignedVector< Light >&,
						drawing::Image*,
						drawing::Image*
					>(&jobTraceLightmap, x, y, sah, gbuffer, surfaces, lights, outputImageDirect, outputImageIndirect));
					jobs.push_back(job);
				}
			}

			log::info << int32_t(jobs.size()) << L" job(s)..." << Endl;
			for (RefArray< Job >::iterator j = jobs.begin(); j != jobs.end(); ++j)
				(*j)->wait();

			// Dilate light-maps.
			log::info << L"Dilating light-maps..." << Endl;
			drawing::DilateFilter dilateFilter;
			outputImageDirect->apply(&dilateFilter);
			outputImageIndirect->apply(&dilateFilter);

			// Blur indirect lighting.
			log::info << L"Convolving indirect lighting..." << Endl;
			outputImageIndirect->apply(drawing::ConvolutionFilter::createGaussianBlur5());

			// Merge light-maps.
			for (int32_t y = 0; y < c_outputHeight; ++y)
			{
				for (int32_t x = 0; x < c_outputWidth; ++x)
				{
					Color4f inA, inB;
					outputImageIndirect->getPixelUnsafe(x, y, inA);
					outputImageDirect->getPixelUnsafe(x, y, inB);
					outputImageDirect->setPixelUnsafe(x, y, inA + inB);
				}
			}

			outputImageIndirect = 0;

			// Save light-map for debugging.
			outputImageDirect->save(illumEntityData->getSeedGuid().format() + L".png");

			log::info << L"Creating resources..." << Endl;

			// Create a texture build step.
			Ref< render::TextureOutput > textureOutput = new render::TextureOutput();
			textureOutput->m_keepZeroAlpha = false;
			textureOutput->m_hasAlpha = false;
			textureOutput->m_ignoreAlpha = true;
			textureOutput->m_linearGamma = true;
			textureOutput->m_systemTexture = true;
			pipelineBuilder->buildOutput(
				textureOutput,
				L"Generated/__Illumination__Texture__" + advanceGuid(illumEntityData->getSeedGuid(), 0).format(),
				advanceGuid(illumEntityData->getSeedGuid(), 0),
				outputImageDirect
			);

			// Modify model materials to use our illumination texture.
			std::vector< model::Material > materials = model->getMaterials();
			materials[0].setLightMap(model::Material::Map(L"__Illumination__", 0, false));
			model->setMaterials(materials);

			// Create a new mesh asset which use the fresh baked illumination texture.
			std::map< std::wstring, Guid > meshMaterialTextures;
			meshMaterialTextures[L"__Illumination__"] = advanceGuid(illumEntityData->getSeedGuid(), 0);

			Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
			outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
			outputMeshAsset->setMaterialTextures(meshMaterialTextures);
			outputMeshAsset->setGenerateOccluder(false);

			pipelineBuilder->buildOutput(
				outputMeshAsset,
				L"Generated/__Illumination__Mesh__" + advanceGuid(illumEntityData->getSeedGuid(), 1).format(),
				advanceGuid(illumEntityData->getSeedGuid(), 1),
				model
			);

			// Then replace mesh used by mesh entity with the illuminated version.
			(*i)->setMesh(resource::Id< mesh::IMesh >(advanceGuid(illumEntityData->getSeedGuid(), 1)));
		}

		// Replace illumination entity data with a plain group.
		Ref< world::GroupEntityData > groupEntityData = new world::GroupEntityData();
		groupEntityData->setName(illumEntityData->getName());
		groupEntityData->setTransform(illumEntityData->getTransform());
		groupEntityData->setEntityData(illumEntityData->getEntityData());
		return groupEntityData;
	}
	else
		return world::EntityPipeline::buildOutput(
			pipelineBuilder,
			sourceAsset
		);
}

	}
}
