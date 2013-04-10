#ifndef traktor_terrain_TerrainEntityEditor_H
#define traktor_terrain_TerrainEntityEditor_H

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

class T_DLLCLASS TerrainEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	static Ref< TerrainEntityEditor > create(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual bool isPickable() const;

	virtual bool queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const;

	virtual void entitySelected(bool selected);

	virtual bool handleCommand(const ui::Command& command);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const;

private:
	TerrainEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);
};

	}
}

#endif	// traktor_terrain_TerrainEntityEditor_H
