#ifndef traktor_animation_AnimatedMeshEntityEditor_H
#define traktor_animation_AnimatedMeshEntityEditor_H

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

class T_DLLCLASS AnimatedMeshEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	AnimatedMeshEntityEditor(scene::SceneEditorContext* context);

	virtual bool handleCommand(
		scene::SceneEditorContext* context,
		scene::EntityAdapter* entityAdapter,
		const ui::Command& command
	);

	virtual void drawGuide(
		scene::SceneEditorContext* context,
		render::PrimitiveRenderer* primitiveRenderer,
		scene::EntityAdapter* entityAdapter
	) const;

private:
	Color m_colorBone;

	void updateSettings(scene::SceneEditorContext* context);
};

	}
}

#endif	// traktor_animation_AnimatedMeshEntityEditor_H
