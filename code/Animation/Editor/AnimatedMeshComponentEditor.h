#pragma once

#include "Physics/Editor/PhysicsRenderer.h"
#include "Scene/Editor/DefaultComponentEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! Animation mesh component editor.
 * \ingroup Animation
 */
class T_DLLCLASS AnimatedMeshComponentEditor : public scene::DefaultComponentEditor
{
	T_RTTI_CLASS;

public:
	AnimatedMeshComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;

private:
	physics::PhysicsRenderer m_physicsRenderer;
};

	}
}

