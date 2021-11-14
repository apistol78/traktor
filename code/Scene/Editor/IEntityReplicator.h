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
class IPipelineDepends;
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

	/*! Add dependencies used by synthesizing entity. */
	virtual bool addDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const world::EntityData* entityData,
		const world::IEntityComponentData* componentData
	) const = 0;

	/*! Create model replica from entity or component data.
	 *
	 * \param pipelineBuilder Pipeline builder.
	 * \param entityData Owner entity data.
	 * \param componentData Component data which we want to represent as a model.
	 * \return Model replica of entity or component.
	 */
	virtual Ref< model::Model > createModel(
		editor::IPipelineBuilder* pipelineBuilder,
		const world::EntityData* entityData,
		const world::IEntityComponentData* componentData
	) const = 0;

	/*! Modify entity or component to use attributes from model.
	 *
	 * \param pipelineBuilder Pipeline builder.
	 * \param entityData Owner entity data.
	 * \param componentData Component data which we want to represent as a model.
	 * \param model Modified model, based of created from createModel method.
	 * \param outputGuid Guid to use for synthesized builds required when modifing output.
	 * \return Replacement entity/component data if necessary, will be replaced in output scene.
	 */
	virtual Ref< Object > modifyOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const world::EntityData* entityData,
		const world::IEntityComponentData* componentData,
		const model::Model* model,
		const Guid& outputGuid
	) const = 0;
};

	}
}