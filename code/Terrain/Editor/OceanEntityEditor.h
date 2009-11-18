#ifndef traktor_terrain_OceanEntityEditor_H
#define traktor_terrain_OceanEntityEditor_H

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

class T_DLLCLASS OceanEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	virtual bool isPickable(
		scene::EntityAdapter* entityAdapter
	) const;

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
};

	}
}

#endif	// traktor_terrain_OceanEntityEditor_H
