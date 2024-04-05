/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/TransformPath.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineCommon.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/Operations/MergeModel.h"
#include "Physics/ShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Shape/Editor/Spline/ControlPointComponentData.h"
#include "Shape/Editor/Spline/SplineComponentData.h"
#include "Shape/Editor/Spline/SplineEntityReplicator.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"
#include "Shape/Editor/Spline/SplineLayerComponentData.h"
#include "World/EntityData.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SplineEntityReplicator", 0, SplineEntityReplicator, world::IEntityReplicator)

bool SplineEntityReplicator::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getPropertyIncludeHash< std::wstring >(L"Pipeline.AssetPath");
	m_modelCachePath = settings->getPropertyIncludeHash< std::wstring >(L"Pipeline.ModelCache.Path");
	return true;
}

TypeInfoSet SplineEntityReplicator::getSupportedTypes() const
{
	return makeTypeInfoSet< SplineComponentData >();
}

RefArray< const world::IEntityComponentData > SplineEntityReplicator::getDependentComponents(
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData
) const
{
	auto group = entityData->getComponent< world::GroupComponentData >();
	if (!group)
		return RefArray< const world::IEntityComponentData >();

	RefArray< const world::IEntityComponentData > dependentComponentData;
	dependentComponentData.push_back(componentData);
	dependentComponentData.push_back(group);
	return dependentComponentData;
}

Ref< model::Model > SplineEntityReplicator::createModel(
	editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData,
	Usage usage
) const
{
	auto splineComponent = entityData->getComponent< SplineComponentData >();
	TransformPath path;

	// Get group component.
	auto group = entityData->getComponent< world::GroupComponentData >();
	if (!group)
	{
		log::error << L"Invalid spline; no control points found." << Endl;
		return nullptr;	
	}	

	// Count number of control points as we need to estimate fraction of each.
	int32_t controlPointCount = 0;
	for (auto entityData : group->getEntityData())
	{
		for (auto componentData : entityData->getComponents())
		{
			if (is_a< ControlPointComponentData >(componentData))
				controlPointCount++;
		}
	}
	if (controlPointCount <= 0)
	{
		log::error << L"Invalid spline; no control points found." << Endl;
		return nullptr;	
	}

	// Create transformation path.
	int32_t controlPointIndex = 0;
	for (auto entityData : group->getEntityData())
	{
		auto controlPointData = entityData->getComponent< ControlPointComponentData >();
		if (!controlPointData)
			continue;

		const Transform T = entityData->getTransform();

		TransformPath::Key k;
		k.T = (float)controlPointIndex / (controlPointCount - 1);
		k.position = T.translation();
		k.orientation = T.rotation().toEulerAngles();
		k.values[0] = controlPointData->getScale();
		k.values[1] = controlPointData->getAutomaticOrientationWeight();
		path.insert(k);

		++controlPointIndex;
	}

	// Generate geometry from path.
	Ref< model::Model > outputModel = new model::Model();
	for (auto componentData : entityData->getComponents())
	{
		auto layerData = dynamic_type_cast< SplineLayerComponentData* >(componentData);
		if (!layerData)
			continue;

		// Create an instance of the layer component.
		Ref< SplineLayerComponent > layer = layerData->createComponent(pipelineCommon->getSourceDatabase(), m_modelCachePath, m_assetPath);
		if (!layer)
			continue;

		// Create model from of the layer.
		Ref< model::Model > layerModel = layer->createModel(path, splineComponent->isClosed(), false);
		if (!layerModel)
			continue;

		// Merge all models into a single model.
		model::MergeModel merge(*layerModel, entityData->getTransform().inverse(), 0.01f);
		merge.apply(*outputModel);
	}

	// Setup visual mesh information.
	//if (usage == Usage::Visual)
	//{
	//	Ref< mesh::MeshAsset > outputMeshAsset = new mesh::MeshAsset();
	//	outputMeshAsset->setMeshType(mesh::MeshAsset::MtStatic);
	//	//outputMeshAsset->setMaterialShaders(materialShaders);
	//	outputModel->setProperty< PropertyObject >(type_name(outputMeshAsset), outputMeshAsset);
	//}

	// Setup collision information.
	if (usage == Usage::Collision)
	{
		const SplineComponentData* splineComponentData = mandatory_non_null_type_cast< const SplineComponentData* >(componentData);

		Ref< physics::MeshAsset > outputShapeMeshAsset = new physics::MeshAsset();
		outputShapeMeshAsset->setCalculateConvexHull(false);
		outputShapeMeshAsset->setMargin(0.0f);
		outputModel->setProperty< PropertyObject >(type_name(outputShapeMeshAsset), outputShapeMeshAsset);

		Ref< physics::ShapeDesc > outputShapeDesc = new physics::ShapeDesc();
		outputShapeDesc->setCollisionGroup(splineComponentData->getCollisionGroup());
		outputShapeDesc->setCollisionMask(splineComponentData->getCollisionMask());
		outputModel->setProperty< PropertyObject >(type_name(outputShapeDesc), outputShapeDesc);

		Ref< physics::StaticBodyDesc > outputBodyDesc = new physics::StaticBodyDesc();
		outputModel->setProperty< PropertyObject >(type_name(outputBodyDesc), outputBodyDesc);
	}

	return outputModel;
}

}
