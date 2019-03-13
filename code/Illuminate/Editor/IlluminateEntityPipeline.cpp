#include "Core/Log/Log.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/SahTree.h"
#include "Core/Math/Triangulator.h"
#include "Core/Math/Winding3.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/Asset.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Illuminate/Editor/IlluminateEntityData.h"
#include "Illuminate/Editor/IlluminateEntityPipeline.h"
#include "Illuminate/Editor/RayTracer.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MaterialShaderGenerator.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/UnwrapUV.h"
#include "Render/Editor/Texture/ProbeProcessor.h"
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
				return nullptr;
			}

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveAllExternal(pipeline, externalEntityData));
			if (!resolvedEntityData)
			{
				log::error << L"IlluminateEntityPipeline failed; Unable to resolve external entity." << Endl;
				return nullptr;
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
	return makeTypeInfoSet< IlluminateEntityData >();
}

bool IlluminateEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	// Add dependencies to mesh material generator fragments.
	mesh::MaterialShaderGenerator().addDependencies(pipelineDepends);

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
	const IlluminateEntityData* sourceIlluminateEntityData = mandatory_non_null_type_cast< const IlluminateEntityData* >(sourceAsset);

	// Flatten entire hierarchy of illuminate entity.
	Ref< IlluminateEntityData > illumEntityData = checked_type_cast< IlluminateEntityData* >(resolveAllExternal(pipelineBuilder, sourceIlluminateEntityData));
	if (!illumEntityData)
	{
		log::error << L"IlluminateEntityPipeline failed; unable to resolve all external entities." << Endl;
		return nullptr;
	}

	// Get all trace entities.
	RefArray< world::ComponentEntityData > lightEntityDatas;
	RefArray< world::ComponentEntityData > meshEntityDatas;
	collectTraceEntities(illumEntityData, lightEntityDatas, meshEntityDatas);

	// Prepare tracer; add lights and models.
	RayTracer tracer;

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
			light.color = Color4f(lightComponentData->getColor());
			light.range = Scalar(0.0f);
			tracer.addLight(light);
		}
		else if (lightComponentData->getLightType() == world::LtPoint)
		{
			light.type = Light::LtPoint;
			light.position = lightEntityData->getTransform().translation().xyz1();
			light.direction = Vector4::zero();
			light.color = Color4f(lightComponentData->getColor());
			light.range = Scalar(lightComponentData->getRange());
			tracer.addLight(light);
		}
		else
			log::warning << L"IlluminateEntityPipeline warning; unsupported light type of light \"" << lightEntityData->getName() << L"\"." << Endl;
	}

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

		model->clear(model::Model::CfColors | model::Model::CfTexCoords | model::Model::CfJoints);
		model::Triangulate().apply(*model);
		model::CleanDuplicates(0.001f).apply(*model);
		model::CleanDegenerate().apply(*model);

		tracer.addModel(model, meshEntityData->getTransform());
	}

	// Finished adding lights and model; need to prepare acceleration structure.
	tracer.prepare();

	// Create the output entity data, all meshes is grouped.
	Ref< world::GroupEntityData > outputEntityData = new world::GroupEntityData();
	outputEntityData->setName(illumEntityData->getName());
	outputEntityData->setTransform(illumEntityData->getTransform());

	// Raytrace lightmap for each mesh.
	for (uint32_t i = 0; i < meshEntityDatas.size(); ++i)
	{
		auto meshEntityData = meshEntityDatas[i];
		T_FATAL_ASSERT(meshEntityData != nullptr);

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

		model::Triangulate().apply(*model);

		uint32_t channel = model->addUniqueTexCoordChannel(L"Illuminate_LightmapUV");
		if (!model::UnwrapUV(channel).apply(*model))
		{
			log::error << L"IlluminateEntityPipeline failed; unable to unwrap UV of model \"" << meshAsset->getFileName().getOriginal() << L"\"." << Endl;
			return nullptr;
		}

		const int32_t c_lightMapWidth = 256;
		const int32_t c_lightMapHeight = 256;

		Ref< drawing::Image > lightmap = new drawing::Image(drawing::PixelFormat::getRGBAF32(), c_lightMapWidth, c_lightMapHeight);
		lightmap->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

		for (const auto& polygon : model->getPolygons())
		{
			AlignedVector< Vector4 > positions;
			AlignedVector< Vector4 > normals;
			Winding2 texCoords;

			// Extract data for polygon.
			for (const auto index : polygon.getVertices())
			{
				const auto& vertex = model->getVertex(index);

				uint32_t positionIndex = vertex.getPosition();
				positions.push_back(model->getPosition(positionIndex));

				uint32_t normalIndex = vertex.getNormal();
				normals.push_back(model->getNormal(normalIndex));

				uint32_t texCoordIndex = vertex.getTexCoord(channel);
				texCoords.points.push_back(model->getTexCoord(texCoordIndex) * Vector2(c_lightMapWidth, c_lightMapHeight));
			}

			// Triangulate winding so we can easily traverse lightmap fragments.
			AlignedVector< Triangulator::Triangle > triangles;
			Triangulator().freeze(texCoords.points, triangles);
			for (const auto& triangle : triangles)
			{
				size_t i0 = triangle.indices[0];
				size_t i1 = triangle.indices[1];
				size_t i2 = triangle.indices[2];

				const Vector4& p0 = positions[i0];
				const Vector4& p1 = positions[i1];
				const Vector4& p2 = positions[i2];

				const Vector4& n0 = normals[i0];
				const Vector4& n1 = normals[i1];
				const Vector4& n2 = normals[i2];

				const Vector2& tc0 = texCoords.points[i0];
				const Vector2& tc1 = texCoords.points[i1];
				const Vector2& tc2 = texCoords.points[i2];

				Aabb2 aabb;
				aabb.contain(tc0);
				aabb.contain(tc1);
				aabb.contain(tc2);

				int32_t x0 = int32_t(aabb.mn.x);
				int32_t x1 = int32_t(aabb.mx.x + 1);
				int32_t y0 = int32_t(aabb.mn.y);
				int32_t y1 = int32_t(aabb.mx.y + 1);

				float denom = (tc1.y - tc2.y) * (tc0.x - tc2.x) + (tc2.x - tc1.x) * (tc0.y - tc2.y);
				float invDenom = 1.0f / denom;

				for (int32_t y = y0; y <= y1; ++y)
				{
					for (int32_t x = x0; x <= x1; ++x)
					{
						Vector2 pt = Vector2(x, y);

						float alpha = ((tc1.y - tc2.y) * (pt.x - tc2.x) + (tc2.x - tc1.x) * (pt.y - tc2.y)) * invDenom;
						float beta = ((tc2.y - tc0.y) * (pt.x - tc2.x) + (tc0.x - tc2.x) * (pt.y - tc2.y)) * invDenom;
						float gamma = 1.0f - alpha - beta;

						if (alpha < 0.0f || beta < 0.0f || gamma < 0.0f)
							continue;

						Vector4 position = (p0 * Scalar(alpha) + p1 * Scalar(beta) + p2 * Scalar(gamma)).xyz1();
						Vector4 normal = (n0 * Scalar(alpha) + n1 * Scalar(beta) + n2 * Scalar(gamma)).xyz0().normalized();

						Color4f direct(0.0f, 0.0f, 0.0f, 0.0f);
						Color4f indirect(0.0f, 0.0f, 0.0f, 0.0f);

						if (illumEntityData->traceDirectLighting())
							direct = tracer.traceDirect(position, normal, 0.8f);

						if (illumEntityData->traceIndirectLighting())
							indirect = tracer.traceIndirect(position, normal, 0.8f);

						lightmap->setPixel(x, y, direct + indirect);
					}
				}
			}
		}

		// Discard alpha.
		lightmap->clearAlpha(1.0f);

		// "Permutate" output ids.
		Guid idLightMap = illumEntityData->getSeedGuid().permutate(i * 10 + 1);
		Guid idMesh = illumEntityData->getSeedGuid().permutate(i * 10 + 2);

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
			L"Generated/__Illumination__Texture__" + idLightMap.format(),
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
		auto materialTextures = meshAsset->getMaterialTextures();
		materialTextures[L"__Illumination__"] = idLightMap;

		Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
		outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
		outputMeshAsset->setMaterialTextures(materialTextures);
		pipelineBuilder->buildOutput(
			outputMeshAsset,
			L"Generated/__Illumination__Mesh__" + idMesh.format(),
			idMesh,
			model
		);

		// Create new mesh entity.
		Ref< world::ComponentEntityData > entityData = new world::ComponentEntityData();
		entityData->setName(meshEntityData->getName());
		entityData->setTransform(meshEntityData->getTransform());
		entityData->setComponent(new mesh::MeshComponentData(
			resource::Id< mesh::IMesh >(idMesh)
		));
		outputEntityData->addEntityData(entityData);
	}

	Guid idExternal = illumEntityData->getSeedGuid().permutate(0);

	pipelineBuilder->buildOutput(
		outputEntityData,
		L"Generated/__Illumination__GroupEntity__" + idExternal.format(),
		idExternal,
		outputEntityData
	);

	Ref< world::ExternalEntityData > externalEntityData = new world::ExternalEntityData();
	externalEntityData->setName(illumEntityData->getName());
	externalEntityData->setTransform(illumEntityData->getTransform());
	externalEntityData->setEntityData(resource::Id< world::EntityData >(idExternal));
	return externalEntityData;
}

	}
}
