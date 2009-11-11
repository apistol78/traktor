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
	T_RTTI_CLASS(PhysicsEntityEditorFactory)

public:
	virtual const TypeSet getEntityDataTypes() const;

	virtual Ref< scene::IEntityEditor > createEntityEditor(scene::SceneEditorContext* context, const Type& entityDataType) const;
};

	}
}

#endif	// traktor_physics_PhysicsEntityEditorFactory_H
