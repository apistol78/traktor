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
#include "Editor/IPipelineCommon.h"
#include "Model/Model.h"
#include "Model/Operations/MergeModel.h"
#include "Shape/Editor/Spline/ControlPointComponentData.h"
#include "Shape/Editor/Spline/SplineEntityData.h"
#include "Shape/Editor/Spline/SplineEntityReplicator.h"
#include "Shape/Editor/Spline/SplineLayerComponentData.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.SplineEntityReplicator", 0, SplineEntityReplicator, world::IEntityReplicator)

bool SplineEntityReplicator::create(const editor::IPipelineSettings* settings)
{
    return true;
}

TypeInfoSet SplineEntityReplicator::getSupportedTypes() const
{
    return makeTypeInfoSet< SplineEntityData >();
}

Ref< model::Model > SplineEntityReplicator::createModel(
	editor::IPipelineCommon* pipelineCommon,
    const world::EntityData* entityData,
    const world::IEntityComponentData* componentData,
	Usage usage
) const
{
	auto splineEntityData = mandatory_non_null_type_cast< const SplineEntityData* >(entityData);
	TransformPath path;

	// Get group component.
	auto group = splineEntityData->getComponent< world::GroupComponentData >();
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

		Transform T = entityData->getTransform();

		TransformPath::Key k;
		k.T = (float)controlPointIndex / (controlPointCount - 1);
		k.position = T.translation();
		k.orientation = T.rotation().toEulerAngles();
		k.values[0] = controlPointData->getScale();
		path.insert(k);

		++controlPointIndex;
	}

	// Create model, add geometry for each layer.
    Ref< model::Model > outputModel = new model::Model();

	for (auto componentData : splineEntityData->getComponents())
	{
		auto layerData = dynamic_type_cast< SplineLayerComponentData* >(componentData);
		if (!layerData)
			continue;

		Ref< model::Model > layerModel = nullptr; // layerData->createModel(pipelineBuilder->getSourceDatabase(), assetPath, path);
		if (!layerModel)
			continue;

		model::MergeModel merge(*layerModel, splineEntityData->getTransform().inverse(), 0.01f);
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
}

    }
}
