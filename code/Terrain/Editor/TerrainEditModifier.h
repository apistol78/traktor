/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4f.h"
#include "Scene/Editor/IModifier.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/Editor/TerrainEditData.h"

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace drawing
	{

class Image;

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
class TerrainComponentData;
class TerrainLayerComponent;

class TerrainEditModifier : public scene::IModifier
{
	T_RTTI_CLASS;

	struct TileMaps
	{
		Ref< render::ITexture > heightMap;
		Ref< render::ITexture > normalMap;
		Ref< render::ITexture > splatMap;
		Ref< render::ITexture > colorMap;
		Ref< render::ITexture > cutMap;
	};

public:
	explicit TerrainEditModifier(scene::SceneEditorContext* context);

	virtual bool activate() override final;

	virtual void deactivate() override final;

	virtual void selectionChanged() override final;

	virtual void buttonDown() override final;

	virtual CursorMovedResult cursorMoved(
		const scene::TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection
	) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual bool begin(
		const scene::TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		int32_t mouseButton
	) override final;

	virtual void apply(
		const scene::TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		const Vector4& screenDelta,
		const Vector4& viewDelta,
		bool snapOverrideEnable
	) override final;

	virtual void end(const scene::TransformChain& transformChain) override final;

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer, bool orthogonal) const override final;

	void setBrush(const TypeInfo& brushType);

	void setFallOff(const std::wstring& fallOff);

	void setStrength(float strength);

	void setColor(const Color4f& color);

	void setMaterial(int32_t material);

	void setAttribute(int32_t attribute);

	void setVisualizeMode(TerrainComponent::VisualizeMode visualizeMode);

	void setFallOffImage(drawing::Image* fallOffImage);

private:
	scene::SceneEditorContext* m_context;
	Ref< scene::EntityAdapter > m_terrainAdapter;
	Ref< TerrainComponent > m_terrainComponent;
	Ref< TerrainComponentData > m_terrainComponentData;
	RefArray< TerrainLayerComponent > m_terrainLayers;
	Ref< db::Instance > m_terrainInstance;
	Ref< db::Instance > m_heightfieldInstance;
	Ref< hf::HeightfieldAsset > m_heightfieldAsset;
	Ref< TerrainEditData > m_data;
	AlignedVector< TileMaps > m_tileMaps;
	Ref< IBrush > m_brush;
	uint32_t m_brushMode;
	Ref< IFallOff > m_fallOff;
	Ref< drawing::Image > m_fallOffImage;
	float m_strength;
	Color4f m_color;
	int32_t m_material;
	int32_t m_attribute;
	TerrainComponent::VisualizeMode m_visualizeMode;
	Vector4 m_center;
	int32_t m_dirtyRegion[4];	//!< Grid region modified by current stroke.
	float m_radius = 4.0f;
	bool m_applied;
	bool m_editBrushSize = false;

	bool begin(bool inverted);

	void apply(const Vector4& center);

	//! Update maps of tiles intersecting grid region; editable tile maps are created first time tile is edited.
	void updateTiles(const int32_t* region, uint32_t brushMode);

	void end();

	void flattenUnderSpline();
};

	}
}

