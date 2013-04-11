#ifndef traktor_terrain_TerrainEditModifier_H
#define traktor_terrain_TerrainEditModifier_H

#include "Core/Misc/AutoPtr.h"
#include "Resource/Proxy.h"
#include "Scene/Editor/IModifier.h"

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace hf
	{

class Heightfield;
class HeightfieldAsset;

	}

	namespace render
	{

class ISimpleTexture;

	}

	namespace scene
	{

class EntityAdapter;
class SceneEditorContext;

	}

	namespace terrain
	{

class IBrush;
class IFallOff;
class TerrainEntity;
class TerrainEntityData;

class TerrainEditModifier : public scene::IModifier
{
	T_RTTI_CLASS;

public:
	TerrainEditModifier(scene::SceneEditorContext* context);

	virtual void selectionChanged();

	virtual bool cursorMoved(
		const scene::TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection
	);

	virtual bool handleCommand(const ui::Command& command);

	virtual bool begin(
		const scene::TransformChain& transformChain,
		int32_t mouseButton
	);

	virtual void apply(
		const scene::TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		const Vector4& screenDelta,
		const Vector4& viewDelta
	);

	virtual void end(const scene::TransformChain& transformChain);

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) const;

	void setStrength(float strength) { m_strength = strength; }

private:
	scene::SceneEditorContext* m_context;
	Ref< terrain::TerrainEntity > m_entity;
	Ref< terrain::TerrainEntityData > m_entityData;
	Ref< db::Instance > m_heightfieldInstance;
	Ref< hf::HeightfieldAsset > m_heightfieldAsset;
	resource::Proxy< hf::Heightfield > m_heightfield;
	AutoArrayPtr< uint8_t > m_normalData;
	Ref< render::ISimpleTexture > m_normalMap;
	Ref< IBrush > m_brush;
	Ref< IFallOff > m_fallOff;
	float m_strength;
	Vector4 m_center;
};

	}
}

#endif	// traktor_terrain_TerrainEditModifier_H
