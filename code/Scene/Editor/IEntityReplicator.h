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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Guid;

	namespace editor
	{

class IPipelineCommon;
class IPipelineSettings;

	}

	namespace model
	{

class Model;

	}

	namespace world
	{

class EntityData;
class GroupComponentData;
class IEntityComponentData;

	}

	namespace scene
	{

/*! Generate model from source entity or component data, such as mesh, terrain, spline, solid etc.
 * \ingroup Scene
 */
class T_DLLCLASS IEntityReplicator : public Object
{
	T_RTTI_CLASS;

public:
	static const wchar_t* VisualMesh;
	static const wchar_t* CollisionMesh;
	static const wchar_t* CollisionShape;
	static const wchar_t* CollisionBody;

	/*! */
	virtual bool create(const editor::IPipelineSettings* settings) = 0;

	/*! */
	virtual TypeInfoSet getSupportedTypes() const = 0;

	/*! Create visual model replica from entity or component data.
	 *
	 * \param pipelineCommon Pipeline common implementation.
	 * \param entityData Owner entity data.
	 * \param componentData Component data which we want to represent as a model.
	 * \return Model replica of entity or component.
	 */
	virtual Ref< model::Model > createVisualModel(
		editor::IPipelineCommon* pipelineCommon,
		const world::EntityData* entityData,
		const world::IEntityComponentData* componentData
	) const = 0;

	/*! Create collision model replica from entity or component data.
	 *
	 * \param pipelineCommon Pipeline common implementation.
	 * \param entityData Owner entity data.
	 * \param componentData Component data which we want to represent as a model.
	 * \return Model replica of entity or component.
	 */
	virtual Ref< model::Model > createCollisionModel(
		editor::IPipelineCommon* pipelineCommon,
		const world::EntityData* entityData,
		const world::IEntityComponentData* componentData
	) const = 0;

	/*! Transform entity and component data. */
	virtual void transform(
		world::EntityData* entityData,
		world::IEntityComponentData* componentData,
		world::GroupComponentData* outputGroup
	) const = 0;
};

	}
}