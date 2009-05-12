#ifndef traktor_animation_PathEntityEditor_H
#define traktor_animation_PathEntityEditor_H

#include "Scene/Editor/DefaultEntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class T_DLLCLASS PathEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS(PathEntityEditor)

public:
	virtual TypeSet getEntityTypes() const;

	virtual void entitySelected(
		scene::SceneEditorContext* context,
		scene::EntityAdapter* entityAdapter,
		bool selected
	) const;

	virtual void applyModifier(
		scene::SceneEditorContext* context,
		scene::EntityAdapter* entityAdapter,
		const Matrix44& viewTransform,
		const Vector2& mouseDelta,
		int mouseButton
	) const;

	virtual bool handleCommand(
		scene::SceneEditorContext* context,
		scene::EntityAdapter* entityAdapter,
		const ui::Command& command
	) const;

	virtual void drawGuide(
		scene::SceneEditorContext* context,
		render::PrimitiveRenderer* primitiveRenderer,
		scene::EntityAdapter* entityAdapter
	) const;
};

	}
}

#endif	// traktor_animation_PathEntityEditor_H
