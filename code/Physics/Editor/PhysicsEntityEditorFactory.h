#ifndef traktor_physics_PhysicsEntityEditorFactory_H
#define traktor_physics_PhysicsEntityEditorFactory_H

#include "Scene/Editor/IEntityEditorFactory.h"

namespace traktor
{
	namespace physics
	{

/*! \brief
 */
class PhysicsEntityEditorFactory : public scene::IEntityEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityDataTypes() const;

	virtual Ref< scene::IEntityEditor > createEntityEditor(scene::SceneEditorContext* context, const TypeInfo& entityDataType) const;
};

	}
}

#endif	// traktor_physics_PhysicsEntityEditorFactory_H
