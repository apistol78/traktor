#pragma once

#include "Scene/Editor/DefaultComponentEditor.h"

namespace traktor
{
	namespace ai
	{

/*! Navigation mesh component editor.
 * \ingroup AI
 */
class NavMeshComponentEditor : public scene::DefaultComponentEditor
{
	T_RTTI_CLASS;

public:
	NavMeshComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;
};

	}
}
