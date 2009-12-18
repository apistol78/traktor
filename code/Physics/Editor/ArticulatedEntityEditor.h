#ifndef traktor_physics_ArticulatedEntityEditor_H
#define traktor_physics_ArticulatedEntityEditor_H

#include "Scene/Editor/DefaultEntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class T_DLLCLASS ArticulatedEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	ArticulatedEntityEditor(
		scene::SceneEditorContext* context
	);

	virtual void drawGuide(
		scene::SceneEditorContext* context,
		render::PrimitiveRenderer* primitiveRenderer,
		scene::EntityAdapter* entityAdapter
	) const;
};

	}
}

#endif	// traktor_physics_ArticulatedEntityEditor_H
