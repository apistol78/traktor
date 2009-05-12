#ifndef traktor_scene_DefaultEntityEditor_H
#define traktor_scene_DefaultEntityEditor_H

#include "Scene/Editor/EntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class T_DLLCLASS DefaultEntityEditor : public EntityEditor
{
	T_RTTI_CLASS(DefaultEntityEditor)

public:
	virtual TypeSet getEntityTypes() const;

	virtual void entitySelected(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter,
		bool selected
	) const;

	virtual void applyModifier(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter,
		const Matrix44& viewTransform,
		const Vector2& mouseDelta,
		int mouseButton
	) const;

	virtual bool handleCommand(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter,
		const ui::Command& command
	) const;

	virtual void drawGuide(
		SceneEditorContext* context,
		render::PrimitiveRenderer* primitiveRenderer,
		EntityAdapter* entityAdapter
	) const;

	virtual bool getStatusText(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter,
		std::wstring& outStatusText
	) const;
};

	}
}

#endif	// traktor_scene_DefaultEntityEditor_H
