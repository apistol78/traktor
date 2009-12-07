#ifndef traktor_terrain_TerrainEntityEditor_H
#define traktor_terrain_TerrainEntityEditor_H

#include "Scene/Editor/DefaultEntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace terrain
	{

class T_DLLCLASS TerrainEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	TerrainEntityEditor(scene::SceneEditorContext* context);

	virtual bool isPickable(
		scene::EntityAdapter* entityAdapter
	) const;

	virtual void entitySelected(
		scene::SceneEditorContext* context,
		scene::EntityAdapter* entityAdapter,
		bool selected
	);

	virtual void applyModifier(
		scene::SceneEditorContext* context,
		scene::EntityAdapter* entityAdapter,
		const Matrix44& viewTransform,
		const Vector4& screenDelta,
		const Vector4& viewDelta,
		const Vector4& worldDelta,
		int mouseButton
	);

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
	bool m_followGround;
	float m_followHeight;
};

	}
}

#endif	// traktor_terrain_TerrainEntityEditor_H
