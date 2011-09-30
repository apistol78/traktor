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

	virtual void entitySelected(bool selected);

	virtual void applyModifier(const ApplyParams& params);

	virtual bool handleCommand(const ui::Command& command);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const;

private:
	float m_time;
};

	}
}

#endif	// traktor_animation_PathEntityEditor_H
