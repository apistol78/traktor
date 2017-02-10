#ifndef traktor_physics_PhysicsComponentEditorFactory_H
#define traktor_physics_PhysicsComponentEditorFactory_H

#include "Scene/Editor/IComponentEditorFactory.h"

namespace traktor
{
	namespace physics
	{

/*! \brief
 */
class PhysicsComponentEditorFactory : public scene::IComponentEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getComponentDataTypes() const T_OVERRIDE T_FINAL;

	virtual Ref< scene::IComponentEditor > createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_physics_PhysicsComponentEditorFactory_H
