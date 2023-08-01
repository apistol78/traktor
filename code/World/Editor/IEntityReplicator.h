/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Guid;

}

namespace traktor::editor
{

class IPipelineCommon;
class IPipelineSettings;

}

namespace traktor::model
{

class Model;

}

namespace traktor::world
{

class EntityData;
class GroupComponentData;
class IEntityComponentData;

}

namespace traktor::world
{

/*! Generate model from source entity or component data, such as mesh, terrain, spline, solid etc.
 * \ingroup World
 */
class T_DLLCLASS IEntityReplicator : public Object
{
	T_RTTI_CLASS;

public:
	enum class Usage
	{
		Visual,
		Collision
	};

	/*! */
	virtual bool create(const editor::IPipelineSettings* settings) = 0;

	/*! */
	virtual TypeInfoSet getSupportedTypes() const = 0;

	/*! Get all components from the entity which the component depend upon to generate geometry.
	 *
	 * \param entityData Owner entity data.
	 * \param componentData Component data which we want to represent as a model.
	 * \return Dependent components.
	 */
	virtual RefArray< const world::IEntityComponentData > getDependentComponents(
		const world::EntityData* entityData,
		const world::IEntityComponentData* componentData
	) const = 0;

	/*! Create model replica from entity or component data.
	 *
	 * \param pipelineCommon Pipeline common implementation.
	 * \param entityData Owner entity data.
	 * \param componentData Component data which we want to represent as a model.
	 * \param usage Usage of replica.
	 * \return Model replica of entity or component.
	 */
	virtual Ref< model::Model > createModel(
		editor::IPipelineCommon* pipelineCommon,
		const world::EntityData* entityData,
		const world::IEntityComponentData* componentData,
		Usage usage
	) const = 0;

	/*! Transform entity and component data. */
	virtual void transform(
		world::EntityData* entityData,
		world::IEntityComponentData* componentData,
		world::GroupComponentData* outputGroup
	) const = 0;
};

}
