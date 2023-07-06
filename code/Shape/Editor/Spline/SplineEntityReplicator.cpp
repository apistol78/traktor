/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/TransformPath.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineCommon.h"
#include "Editor/IPipelineSettings.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/Operations/MergeModel.h"
#include "Shape/Editor/Spline/ControlPointComponentData.h"
#include "Shape/Editor/Spline/SplineComponentData.h"
#include "Shape/Editor/Spline/SplineEntityReplicator.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"
#include "Shape/Editor/Spline/SplineLayerComponentData.h"
#include "World/EntityData.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor
{
	namespace shape
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

Ref< model::Model > SplineEntityReplicator::createModel(
	editor::IPipelineCommon* pipelineCommon,
	const world::EntityData* entityData,
	const world::IEntityComponentData* componentData,
	Usage usage
) const
{
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
		Ref< model::ModelCache > modelCache = new model::ModelCache(m_modelCachePath);
		Ref< SplineLayerComponent > layer = layerData->createComponent(pipelineCommon->getSourceDatabase(), modelCache, m_assetPath);
		if (!layer)
			continue;

		// Create model from of the layer.
		Ref< model::Model > layerModel = layer->createModel(path);
		if (!layerModel)
			continue;

		// Merge all models into a single model.
		model::MergeModel merge(*layerModel, entityData->getTransform().inverse(), 0.01f);
		merge.apply(*outputModel);
	}

	return outputModel;
}

void SplineEntityReplicator::transform(
	world::EntityData* entityData,
	world::IEntityComponentData* componentData,
	world::GroupComponentData* outputGroup
) const
{
	// Remove all components from this entity since no spline components are suppose to be part of runtime.
	RefArray< world::IEntityComponentData > components = entityData->getComponents();
	for (auto component : components)
		entityData->removeComponent(component);
}

	}
}
