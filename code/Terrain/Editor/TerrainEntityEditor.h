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
	namespace hf
	{

class IBrush;
class HeightfieldCompositor;
class Region;

	}

	namespace terrain
	{

class T_DLLCLASS TerrainEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	static Ref< TerrainEntityEditor > create(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual bool queryRay(
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		Scalar& outDistance
	) const;

	virtual void cursorMoved(const ApplyParams& params);

	virtual void beginModifier(const ApplyParams& params);

	virtual void applyModifier(const ApplyParams& params);

	virtual void endModifier(const ApplyParams& params);

	virtual bool handleCommand(const ui::Command& command);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const;

private:
	Vector4 m_strokeLast;
	hf::HeightfieldCompositor* m_compositor;
	Ref< hf::IBrush > m_brush;
	int32_t m_brushType;
	float m_brushRadius;

	TerrainEntityEditor(
		scene::SceneEditorContext* context,
		scene::EntityAdapter* entityAdapter,
		hf::HeightfieldCompositor* compositor
	);

	void updateHeightfield(bool patches, bool normals, bool heights, const hf::Region& dirtyRegion);
};

	}
}

#endif	// traktor_terrain_TerrainEntityEditor_H
