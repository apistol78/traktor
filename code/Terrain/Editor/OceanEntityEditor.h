#ifndef traktor_terrain_OceanEntityEditor_H
#define traktor_terrain_OceanEntityEditor_H

#include "Scene/Editor/DefaultEntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace terrain
	{

class T_DLLCLASS OceanEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	OceanEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual bool handleCommand(const ui::Command& command);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const;
};

	}
}

#endif	// traktor_terrain_OceanEntityEditor_H
