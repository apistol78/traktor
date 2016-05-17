#ifndef traktor_animation_AnimatedMeshEntityEditor_H
#define traktor_animation_AnimatedMeshEntityEditor_H

#include "Scene/Editor/DefaultEntityEditor.h"

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

class T_DLLCLASS AnimatedMeshEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	AnimatedMeshEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const T_OVERRIDE T_FINAL;

private:
	Color4ub m_colorBone;

	void updateSettings();
};

	}
}

#endif	// traktor_animation_AnimatedMeshEntityEditor_H
