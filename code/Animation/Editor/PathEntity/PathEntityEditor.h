#ifndef traktor_animation_PathEntityEditor_H
#define traktor_animation_PathEntityEditor_H

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

class T_DLLCLASS PathEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	PathEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual void entitySelected(bool selected) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const T_OVERRIDE T_FINAL;

private:
	float m_time;
};

	}
}

#endif	// traktor_animation_PathEntityEditor_H
