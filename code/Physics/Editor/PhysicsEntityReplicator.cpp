/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Editor/PhysicsEntityReplicator.h"

#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineCommon.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Editor/ConvertHeightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Physics/BoxShapeDesc.h"
#include "Physics/CylinderShapeDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "World/EntityData.h"

#include <cmath>

namespace traktor::physics
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsEntityReplicator", 0, PhysicsEntityReplicator, world::IEntityReplicator)

bool PhysicsEntityReplicator::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath");
	m_modelCachePath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.ModelCache.Path");
	return true;
}

TypeInfoSet PhysicsEntityReplicator::getSupportedTypes() const
{
	return makeTypeInfoSet< RigidBodyComponentData >();
}

RefArray< const world::IEntityComponentData > PhysicsEntityReplicator::getDependentComponents(
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData) const
{
	RefArray< const world::IEntityComponentData > dependentComponentData;
	dependentComponentData.push_back(componentData);
	return dependentComponentData;
}

Ref< model::Model > PhysicsEntityReplicator::createModel(
	editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData,
	Usage usage) const
{
	if (usage != Usage::Collision)
		return nullptr;

	const RigidBodyComponentData* rigidBodyComponentData = mandatory_non_null_type_cast< const RigidBodyComponentData* >(componentData);

	auto bodyDesc = dynamic_type_cast< const StaticBodyDesc* >(rigidBodyComponentData->getBodyDesc());
	if (!bodyDesc)
		return nullptr;

	auto boxShape = dynamic_type_cast< const BoxShapeDesc* >(rigidBodyComponentData->getBodyDesc()->getShape());
	if (boxShape)
	{
		Vector4 vertices[8];
		Aabb3(-boxShape->getExtent(), boxShape->getExtent()).getExtents(vertices);

		Ref< model::Model > m = new model::Model();

		for (uint32_t i = 0; i < 6; ++i)
		{
			const Vector4* normals = Aabb3::getNormals();
			const int* face = Aabb3::getFaces() + i * 4;

			uint32_t vi[4];
			uint32_t ni = m->addUniqueNormal(normals[i]);

			for (int32_t j = 0; j < 4; ++j)
			{
				const Vector4& P = vertices[face[3 - j]];

				model::Vertex vx;
				vx.setPosition(m->addUniquePosition(P));
				vx.setNormal(ni);
				vi[j] = m->addUniqueVertex(vx);
			}

			model::Polygon pol;
			pol.setNormal(ni);
			pol.addVertex(vi[0]);
			pol.addVertex(vi[1]);
			pol.addVertex(vi[2]);
			pol.addVertex(vi[3]);
			m->addPolygon(pol);
		}

		return m;
	}

	auto cylinderShape = dynamic_type_cast< const CylinderShapeDesc* >(rigidBodyComponentData->getBodyDesc()->getShape());
	if (cylinderShape)
	{
		const float radius = cylinderShape->getRadius();
		const float halfLength = cylinderShape->getLength() * 0.5f;
		const int32_t segments = 32;

		Ref< model::Model > m = new model::Model();

		for (int32_t i = 0; i < segments; ++i)
		{
			const float a0 = (float)i / segments * TWO_PI;
			const float a1 = (float)(i + 1) / segments * TWO_PI;
			const float am = ((float)i + 0.5f) / segments * TWO_PI;

			const Vector4 P[] = {
				Vector4(radius * std::cos(a0), radius * std::sin(a0), -halfLength, 1.0f),
				Vector4(radius * std::cos(a0), radius * std::sin(a0),  halfLength, 1.0f),
				Vector4(radius * std::cos(a1), radius * std::sin(a1),  halfLength, 1.0f),
				Vector4(radius * std::cos(a1), radius * std::sin(a1), -halfLength, 1.0f)
			};

			const uint32_t ni = m->addUniqueNormal(Vector4(std::cos(am), std::sin(am), 0.0f, 0.0f));

			model::Polygon pol;
			pol.setNormal(ni);
			for (int32_t j = 0; j < 4; ++j)
			{
				model::Vertex vx;
				vx.setPosition(m->addUniquePosition(P[j]));
				vx.setNormal(ni);
				pol.addVertex(m->addUniqueVertex(vx));
			}
			m->addPolygon(pol);
		}

		const uint32_t topNormal = m->addUniqueNormal(Vector4(0.0f, 0.0f, 1.0f, 0.0f));
		const uint32_t bottomNormal = m->addUniqueNormal(Vector4(0.0f, 0.0f, -1.0f, 0.0f));

		model::Polygon top;
		top.setNormal(topNormal);

		model::Polygon bottom;
		bottom.setNormal(bottomNormal);

		for (int32_t i = 0; i < segments; ++i)
		{
			const float at = (float)(segments - i) / segments * TWO_PI;
			model::Vertex tv;
			tv.setPosition(m->addUniquePosition(Vector4(radius * std::cos(at), radius * std::sin(at), halfLength, 1.0f)));
			tv.setNormal(topNormal);
			top.addVertex(m->addUniqueVertex(tv));

			const float ab = (float)i / segments * TWO_PI;
			model::Vertex bv;
			bv.setPosition(m->addUniquePosition(Vector4(radius * std::cos(ab), radius * std::sin(ab), -halfLength, 1.0f)));
			bv.setNormal(bottomNormal);
			bottom.addVertex(m->addUniqueVertex(bv));
		}

		m->addPolygon(top);
		m->addPolygon(bottom);

		return m;
	}

	auto meshShape = dynamic_type_cast< const MeshShapeDesc* >(rigidBodyComponentData->getBodyDesc()->getShape());
	if (meshShape)
	{
		// Get referenced mesh asset.
		Ref< const physics::MeshAsset > meshAsset = pipelineCommon->getObjectReadOnly< physics::MeshAsset >(meshShape->getMesh());
		if (!meshAsset)
			return nullptr;

		// Read source model.
		Path filePath = FileSystem::getInstance().getAbsolutePath(Path(m_assetPath) + meshAsset->getFileName());
		Ref< model::Model > shapeModel = model::ModelCache::getInstance().getMutable(m_modelCachePath, filePath, meshAsset->getImportFilter());
		if (!shapeModel)
			return nullptr;

		// Attach information about the collision shape into the model.
		Ref< physics::MeshAsset > outputShapeMeshAsset = new physics::MeshAsset();
		outputShapeMeshAsset->setCalculateConvexHull(false);
		outputShapeMeshAsset->setMargin(meshAsset->getMargin());
		outputShapeMeshAsset->setMaterials(meshAsset->getMaterials());
		shapeModel->setProperty< PropertyObject >(type_name(outputShapeMeshAsset), outputShapeMeshAsset);

		Ref< physics::ShapeDesc > outputShapeDesc = new physics::ShapeDesc();
		outputShapeDesc->setCollisionGroup(meshShape->getCollisionGroup());
		outputShapeDesc->setCollisionMask(meshShape->getCollisionMask());
		outputShapeDesc->setMaterial(meshShape->getMaterial());
		shapeModel->setProperty< PropertyObject >(type_name(outputShapeDesc), outputShapeDesc);

		Ref< physics::StaticBodyDesc > outputBodyDesc = new physics::StaticBodyDesc();
		outputBodyDesc->setFriction(bodyDesc->getFriction());
		outputBodyDesc->setRestitution(bodyDesc->getRestitution());
		shapeModel->setProperty< PropertyObject >(type_name(outputBodyDesc), outputBodyDesc);

		return shapeModel;
	}

	auto heightfieldShape = dynamic_type_cast< const HeightfieldShapeDesc* >(rigidBodyComponentData->getBodyDesc()->getShape());
	if (heightfieldShape)
	{
		Ref< db::Instance > heightfieldAssetInstance = pipelineCommon->getSourceDatabase()->getInstance(heightfieldShape->getHeightfield());
		if (!heightfieldAssetInstance)
			return nullptr;

		Ref< const hf::HeightfieldAsset > heightfieldAsset = heightfieldAssetInstance->getObject< const hf::HeightfieldAsset >();
		if (!heightfieldAsset)
			return nullptr;

		Ref< IStream > sourceData = heightfieldAssetInstance->readData(L"Data");
		if (!sourceData)
			return nullptr;

		Ref< hf::Heightfield > heightfield = hf::HeightfieldFormat().read(
			sourceData,
			heightfieldAsset->getWorldExtent());
		if (!heightfield)
			return nullptr;

		safeClose(sourceData);

		return hf::ConvertHeightfield().convert(heightfield, 4);
	}

	return nullptr;
}

}
