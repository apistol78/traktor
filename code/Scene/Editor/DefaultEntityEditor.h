#ifndef traktor_scene_DefaultEntityEditor_H
#define traktor_scene_DefaultEntityEditor_H

#include "Core/Math/Color4ub.h"
#include "Scene/Editor/IEntityEditor.h"

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

class T_DLLCLASS DefaultEntityEditor : public IEntityEditor
{
	T_RTTI_CLASS;

public:
	DefaultEntityEditor(SceneEditorContext* context);

	virtual bool isPickable(
		EntityAdapter* entityAdapter
	) const;

	virtual void entitySelected(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter,
		bool selected
	);

	virtual void beginModifier(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter
	);

	virtual void applyModifier(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter,
		const Matrix44& viewTransform,
		const Vector4& screenDelta,
		const Vector4& viewDelta,
		const Vector4& worldDelta,
		int mouseButton
	);

	virtual void endModifier(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter
	);

	virtual bool handleCommand(
		SceneEditorContext* context,
		EntityAdapter* entityAdapter,
		const ui::Command& command
	);

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

private:
	bool m_inModify;
	Color4ub m_colorBoundingBox;
	Color4ub m_colorBoundingBoxSel;
	Color4ub m_colorBoundingBoxFaceSel;
	Color4ub m_colorSnap;

	void updateSettings(SceneEditorContext* context);
};

	}
}

#endif	// traktor_scene_DefaultEntityEditor_H
