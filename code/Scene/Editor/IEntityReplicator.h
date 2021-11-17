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

class IPipelineBuilder;
class IPipelineSettings;

	}

	namespace model
	{

class Model;

	}

	namespace world
	{

class EntityData;
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
	/*! */
	virtual bool create(const editor::IPipelineSettings* settings) = 0;

	/*! */
	virtual TypeInfoSet getSupportedTypes() const = 0;

	/*! Create visual model replica from entity or component data.
	 *
	 * \param pipelineBuilder Pipeline builder.
	 * \param entityData Owner entity data.
	 * \param componentData Component data which we want to represent as a model.
	 * \return Model replica of entity or component.
	 */
	virtual Ref< model::Model > createVisualModel(
		editor::IPipelineBuilder* pipelineBuilder,
		const world::EntityData* entityData,
		const world::IEntityComponentData* componentData
	) const = 0;

	/*! Create collision model replica from entity or component data.
	 *
	 * \param pipelineBuilder Pipeline builder.
	 * \param entityData Owner entity data.
	 * \param componentData Component data which we want to represent as a model.
	 * \return Model replica of entity or component.
	 */
	virtual Ref< model::Model > createCollisionModel(
		editor::IPipelineBuilder* pipelineBuilder,
		const world::EntityData* entityData,
		const world::IEntityComponentData* componentData
	) const = 0;
};

	}
}