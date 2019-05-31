#pragma once

#include "Scene/Editor/IComponentEditorFactory.h"

namespace traktor
{
	namespace ai
	{

/*! \brief Navigation mesh entity scene editor factory.
 * \ingroup AI
 */
class NavMeshComponentEditorFactory : public scene::IComponentEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getComponentDataTypes() const override final;

	virtual Ref< scene::IComponentEditor > createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const override final;
};

	}
}

