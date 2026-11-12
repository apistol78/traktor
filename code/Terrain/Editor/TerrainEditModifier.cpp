/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/RefArray.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Render/IRenderSystem.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Shape/Editor/Spline/SplineComponent.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainComponentData.h"
#include "Terrain/TerrainLayerComponent.h"
#include "Terrain/Editor/ColorBrush.h"
#include "Terrain/Editor/CutBrush.h"
#include "Terrain/Editor/ElevateBrush.h"
#include "Terrain/Editor/FlattenBrush.h"
#include "Terrain/Editor/ImageFallOff.h"
#include "Terrain/Editor/AttributeBrush.h"
#include "Terrain/Editor/MaterialBrush.h"
#include "Terrain/Editor/NoiseBrush.h"
#include "Terrain/Editor/SharpFallOff.h"
#include "Terrain/Editor/SmoothBrush.h"
#include "Terrain/Editor/SmoothFallOff.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Terrain/Editor/TerrainEditModifier.h"
#include "Ui/Application.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

template < typename Visitor >
void line_dda(float x0, float y0, float x1, float y1, const Visitor& visitor)
{
	float dx = x1 - x0;
	float dy = y1 - y0;

	float dln = std::sqrt(dx * dx + dy * dy);
	if (dln > FUZZY_EPSILON)
	{
		dx /= dln;
		dy /= dln;
	}

	float x = std::floor(x0);
	float y = std::floor(y0);

	float stepx, stepy;
	float cbx, cby;

	if (dx > 0.0f)
	{
		stepx = 1.0f;
		cbx = x + 1.0f;
	}
	else
	{
		stepx = -1.0f;
		cbx = x;
	}

	if (dy > 0.0f)
	{
		stepy = 1.0f;
		cby = y + 1.0f;
	}
	else
	{
		stepy = -1.0f;
		cby = y;
	}

	float tmaxx, tmaxy;
	float tdeltax, tdeltay;
	float rxr, ryr;

	if (std::abs(dx) > FUZZY_EPSILON)
	{
		rxr = 1.0f / dx;
		tmaxx = (cbx - x0) * rxr;
		tdeltax = stepx * rxr;
	}
	else
	{
		tmaxx = std::numeric_limits< float >::max();
		tdeltax = 0.0f;
	}

	if (std::abs(dy) > FUZZY_EPSILON)
	{
		ryr = 1.0f / dy;
		tmaxy = (cby - y0) * ryr;
		tdeltay = stepy * ryr;
	}
	else
	{
		tmaxy = std::numeric_limits< float >::max();
		tdeltay = 0.0f;
	}

	int32_t ix1 = int32_t(x1);
	int32_t iy1 = int32_t(y1);

	for (int32_t i = 0; i < 10000; ++i)
	{
		int32_t ix = int32_t(x);
		int32_t iy = int32_t(y);

		visitor(ix, iy);

		if (ix == ix1 && iy == iy1)
			break;

		if (tmaxx < tmaxy)
		{
			x += stepx;
			tmaxx += tdeltax;
		}
		else
		{
			y += stepy;
			tmaxy += tdeltay;
		}
	}
}

enum class TileMap
{
	Heights,
	Normals,
	Splat,
	Color,
	Cuts
};

Ref< render::ITexture > createTileMap(render::IRenderSystem* renderSystem, int32_t size, render::TextureFormat format)
{
	render::SimpleTextureCreateDesc desc = {};
	desc.width = size;
	desc.height = size;
	desc.mipCount = 1;
	desc.format = format;
	desc.sRGB = false;
	desc.immutable = false;
	return renderSystem->createSimpleTexture(desc, T_FILE_LINE_W);
}

//! Fill region, x0, z0, x1, z1 inclusive in tile coordinates, of tile map from edited data; samples beyond terrain are clamped.
void fillTileMap(render::ITexture* tileMap, TileMap map, TerrainEditData* data, int32_t tileX, int32_t tileZ, const int32_t* region)
{
	const int32_t size = data->getSize();

	render::ITexture::Lock lock;
	if (!tileMap->lock(0, 0, lock))
		return;

	for (int32_t z = region[1]; z <= region[3]; ++z)
	{
		const int32_t gz = std::min(tileZ + z, size - 1);
		uint8_t* row = (uint8_t*)lock.bits + z * lock.pitch;

		for (int32_t x = region[0]; x <= region[2]; ++x)
		{
			const int32_t gx = std::min(tileX + x, size - 1);

			if (map == TileMap::Heights)
				((float*)row)[x] = data->getGridHeightNearest(gx, gz);
			else if (map == TileMap::Normals)
			{
				const Vector4 normal = data->normalAt((float)gx, (float)gz) * Vector4(0.5f, 0.5f, 0.5f, 0.0f) + Vector4(0.5f, 0.5f, 0.5f, 0.0f);
				uint8_t* ptr = row + x * 4;
				ptr[0] = uint8_t(normal.x() * 255);
				ptr[1] = uint8_t(normal.z() * 255);
				ptr[2] = uint8_t(normal.y() * 255);
				ptr[3] = 0;
			}
			else if (map == TileMap::Splat || map == TileMap::Color)
			{
				Color4f color(0.0f, 0.0f, 0.0f, 0.0f);
				if (map == TileMap::Splat)
					data->getSplat(gx, gz, color);
				else
					data->getColor(gx, gz, color);

				float rgba[4];
				color.saturated().storeUnaligned(rgba);

				uint8_t* ptr = row + x * 4;
				for (int32_t i = 0; i < 4; ++i)
					ptr[i] = uint8_t(rgba[i] * 255.0f + 0.5f);
			}
			else
				row[x] = data->getGridCut(gx, gz) ? 0xff : 0x00;
		}
	}

	const render::ITexture::Region dirty = { region[0], region[1], region[2] - region[0] + 1, region[3] - region[1] + 1 };
	tileMap->unlock(0, 0, dirty);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEditModifier", TerrainEditModifier, scene::IModifier)

TerrainEditModifier::TerrainEditModifier(scene::SceneEditorContext* context)
:	m_context(context)
,	m_brushMode(0)
,	m_strength(1.0f)
,	m_color(Color4f(0.5f, 0.5f, 0.5f, 1.0f))
,	m_material(0)
,	m_attribute(0)
,	m_center(Vector4::zero())
,	m_applied(false)
{
}

bool TerrainEditModifier::activate()
{
	db::Database* sourceDatabase = m_context->getEditor()->getSourceDatabase();
	T_ASSERT(sourceDatabase);

	// Call deactivate again to make sure everything is fresh.
	deactivate();

	// Get terrain component from scene.
	RefArray< scene::EntityAdapter > entityAdapters;
	if (m_context->findAdaptersOfType(
		type_of< TerrainComponent >(),
		entityAdapters,
		scene::SceneEditorContext::GfDefault | scene::SceneEditorContext::GfNoExternalChild
	) != 1)
	{
		// Must exist only one terrain in scene.
		return false;
	}

	m_terrainAdapter = entityAdapters[0];
	m_terrainComponent = m_terrainAdapter->getComponent< TerrainComponent >();
	m_terrainComponentData = m_terrainAdapter->getComponentData< TerrainComponentData >();

	// Ensure we've both component and it's data.
	if (!m_terrainComponent || !m_terrainComponentData)
	{
		m_terrainComponent = nullptr;
		m_terrainComponentData = nullptr;
		return false;
	}

	// Get terrain layer components.
	for (auto component : entityAdapters[0]->getComponents())
	{
		if (auto terrainLayer = dynamic_type_cast< TerrainLayerComponent* >(component))
			m_terrainLayers.push_back(terrainLayer);
	}

	// Source data is loaded tile by tile when edited.
	m_terrainInstance = sourceDatabase->getInstance(m_terrainComponentData->getTerrain());
	Ref< const TerrainAsset > terrainAsset = m_terrainInstance ? m_terrainInstance->getObject< TerrainAsset >() : nullptr;
	m_heightfieldInstance = terrainAsset ? sourceDatabase->getInstance(terrainAsset->getHeightfield()) : nullptr;
	Ref< const hf::HeightfieldAsset > heightfieldAsset = m_heightfieldInstance ? m_heightfieldInstance->getObject< hf::HeightfieldAsset >() : nullptr;

	m_data = new TerrainEditData();
	if (!heightfieldAsset || !m_data->create(m_heightfieldInstance, m_terrainInstance, heightfieldAsset))
	{
		deactivate();
		return false;
	}

	// Create default brush; try set same brush type as before.
	if (m_brush)
		setBrush(type_of(m_brush));
	if (!m_brush)
		m_brush = new ElevateBrush(m_data);
	if (!m_fallOff)
		m_fallOff = new SmoothFallOff();

	// Set visualize mode.
	m_terrainComponent->setVisualizeMode(m_visualizeMode);
	return true;
}

void TerrainEditModifier::deactivate()
{
	m_terrainAdapter = nullptr;
	m_terrainComponent = nullptr;
	m_terrainComponentData = nullptr;
	m_terrainLayers.resize(0);
	m_terrainInstance = nullptr;
	m_heightfieldInstance = nullptr;
	m_heightfieldAsset = nullptr;
	m_data = nullptr;
	m_tileMaps.clear();
}

void TerrainEditModifier::selectionChanged()
{
}

void TerrainEditModifier::buttonDown()
{
	m_editBrushSize = false;
}

scene::IModifier::CursorMovedResult TerrainEditModifier::cursorMoved(
	const scene::TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection
)
{
	if (!m_terrainComponent || !m_data)
		return { false, false };

	const Vector4 lastCenter = m_center;
	bool hot = false;
	bool redraw = false;

	Scalar distance;
	if (m_terrainComponent->queryRay(
		worldRayOrigin,
		worldRayDirection,
		distance
	))
	{
		if (!m_editBrushSize)
		{
			m_center = (worldRayOrigin + worldRayDirection * distance).xyz1();
			redraw = m_center != lastCenter;
		}
		else
		{
			const Vector4 currentCenter = (worldRayOrigin + worldRayDirection * distance).xyz1();
			const Scalar distance = (currentCenter - m_center).length();
			m_radius = float(distance);
			redraw = true;
		}
		hot = true;
	}
	else
	{
		m_center = Vector4::zero();
		redraw = m_center != lastCenter;
	}

	return { hot, redraw };
}

bool TerrainEditModifier::handleCommand(const ui::Command& command)
{
	if (command == L"Terrain.Editor.EditBrushSize")
	{
		m_editBrushSize = true;
		return true;
	}
	else if (command == L"Terrain.Editor.FlattenUnderSpline")
	{
		flattenUnderSpline();
		return true;
	}
	else
		return false;
}

bool TerrainEditModifier::begin(
	const scene::TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	int32_t mouseButton
)
{
	if (m_editBrushSize)
		return false;

	const bool inverted = (ui::Application::getInstance()->getEventLoop()->getAsyncKeyState() & ui::KsControl) != 0;
	return begin(inverted);
}

void TerrainEditModifier::apply(
	const scene::TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	const Vector4& screenDelta,
	const Vector4& viewDelta,
	bool snapOverrideEnable
)
{
	if (!m_terrainComponent || !m_data || m_center.w() <= FUZZY_EPSILON)
		return;

	Vector4 center = Vector4::zero();
	Scalar distance;

	if (m_applied)
	{
		// Find furthest intersection which allows for editing around hills etc.
		Scalar minD = Scalar(std::numeric_limits< float >::max());
		Vector4 step = worldRayOrigin;
		bool found = false;

		for (;;)
		{
			step += worldRayDirection * 0.01_simd;
			if (!m_terrainComponent->queryRay(
				step,
				worldRayDirection,
				distance
			))
				break;

			step = (step + worldRayDirection * distance).xyz1();

			const Scalar d = (step - m_center).xyz0().length();
			if (d < minD)
			{
				center = step;
				minD = d;
				found = true;
			}
		}

		if (!found)
			return;
	}
	else
	{
		if (!m_terrainComponent->queryRay(
			worldRayOrigin,
			worldRayDirection,
			distance
		))
			return;

		center = (worldRayOrigin + worldRayDirection * distance).xyz1();
	}

	apply(center);
}

void TerrainEditModifier::end(const scene::TransformChain& transformChain)
{
	end();
}

void TerrainEditModifier::draw(render::PrimitiveRenderer* primitiveRenderer, bool orthogonal) const
{
	if (!m_terrainComponent || !m_data || m_center.w() <= FUZZY_EPSILON)
		return;

	primitiveRenderer->drawSolidPoint(m_center, 8, Color4ub(255, 0, 0, 255));
	primitiveRenderer->pushDepthState(false, false, false);

	float x0 = m_center.x() + cosf(0.0f) * m_radius;
	float z0 = m_center.z() + sinf(0.0f) * m_radius;
	float y0 = m_terrainComponent->getWorldHeight(x0, z0);

	for (int32_t i = 1; i <= 32; ++i)
	{
		const float a = TWO_PI * i / 32.0f;
		const float x1 = m_center.x() + cosf(a) * m_radius;
		const float z1 = m_center.z() + sinf(a) * m_radius;
		const float y1 = m_terrainComponent->getWorldHeight(x1, z1);

		primitiveRenderer->drawLine(
			Vector4(x0, y0 + FUZZY_EPSILON, z0, 1.0f),
			Vector4(x1, y1 + FUZZY_EPSILON, z1, 1.0f),
			2.0f,
			Color4ub(255, 0, 0, 180)
		);

		x0 = x1;
		z0 = z1;
		y0 = y1;
	}

	primitiveRenderer->popDepthState();
}

void TerrainEditModifier::setBrush(const TypeInfo& brushType)
{
	if (is_type_a< ColorBrush >(brushType))
		m_brush = new ColorBrush(m_data);
	else if (is_type_a< CutBrush >(brushType))
		m_brush = new CutBrush(m_data);
	else if (is_type_a< ElevateBrush >(brushType))
		m_brush = new ElevateBrush(m_data);
	else if (is_type_a< FlattenBrush >(brushType))
		m_brush = new FlattenBrush(m_data);
	else if (is_type_a< MaterialBrush >(brushType))
		m_brush = new MaterialBrush(m_data);
	else if (is_type_a< NoiseBrush >(brushType))
		m_brush = new NoiseBrush(m_data);
	else if (is_type_a< SmoothBrush >(brushType))
		m_brush = new SmoothBrush(m_data);
	else if (is_type_a< AttributeBrush >(brushType))
		m_brush = new AttributeBrush(m_data);
}

void TerrainEditModifier::setFallOff(const std::wstring& fallOff)
{
	if (fallOff == L"Terrain.Editor.SmoothFallOff")
		m_fallOff = new SmoothFallOff();
	else if (fallOff == L"Terrain.Editor.SharpFallOff")
		m_fallOff = new SharpFallOff();
	else if (fallOff == L"Terrain.Editor.ImageFallOff")
		m_fallOff = new ImageFallOff(m_fallOffImage);
}

void TerrainEditModifier::setStrength(float strength)
{
	m_strength = strength;
}

void TerrainEditModifier::setColor(const Color4f& color)
{
	m_color = color;
	m_color.setAlpha(Scalar(1.0f));
}

void TerrainEditModifier::setMaterial(int32_t material)
{
	m_material = material;
}

void TerrainEditModifier::setAttribute(int32_t attribute)
{
	m_attribute = attribute;
}

void TerrainEditModifier::setVisualizeMode(TerrainComponent::VisualizeMode visualizeMode)
{
	m_visualizeMode = visualizeMode;
	if (m_terrainComponent)
		m_terrainComponent->setVisualizeMode(m_visualizeMode);
}

void TerrainEditModifier::setFallOffImage(drawing::Image* fallOffImage)
{
	m_fallOffImage = fallOffImage;
}

bool TerrainEditModifier::begin(bool inverted)
{
	if (!m_data)
		return false;

	m_context->setPlaying(false);

	const int32_t gridRadius = int32_t(m_data->getSize() * m_radius / m_data->getWorldExtent().x());

	float gx, gz;
	m_data->worldToGrid(m_center.x(), m_center.z(), gx, gz);

	IBrush::State state;
	state.radius = gridRadius;
	state.falloff = m_fallOff;
	state.strength = m_strength * (inverted ? -1.0f : 1.0f);
	state.color = m_color;
	state.material = m_material;
	state.attribute = m_attribute;

	m_brushMode = m_brush->begin(gx, gz, state);

	m_dirtyRegion[0] =
	m_dirtyRegion[1] =
	m_dirtyRegion[2] =
	m_dirtyRegion[3] = 0;
	m_applied = false;
	return true;
}

void TerrainEditModifier::apply(const Vector4& center)
{
	if (!m_applied)
		m_center = center;

	float gx0, gz0;
	float gx1, gz1;

	m_data->worldToGrid(m_center.x(), m_center.z(), gx0, gz0);
	m_data->worldToGrid(center.x(), center.z(), gx1, gz1);

	// Apply brush along entire line.
	line_dda(gx0, gz0, gx1, gz1, [=](int32_t x, int32_t y) {
		m_brush->apply(x, y);
	});

	m_center = center;

	const int32_t size = m_data->getSize();

	// Calculate region which needs to be updated.
	const int32_t gridRadius = int32_t(size * m_radius / m_data->getWorldExtent().x());

	int32_t mnx = (int32_t)min(std::floor(gx0) - gridRadius, std::floor(gx1) - gridRadius);
	int32_t mxx = (int32_t)max(std::ceil(gx0) + gridRadius, std::ceil(gx1) + gridRadius);
	int32_t mnz = (int32_t)min(std::floor(gz0) - gridRadius, std::floor(gz1) - gridRadius);
	int32_t mxz = (int32_t)max(std::ceil(gz0) + gridRadius, std::ceil(gz1) + gridRadius);

	mnx = clamp(mnx, 0, size - 1);
	mxx = clamp(mxx, 0, size - 1);
	mnz = clamp(mnz, 0, size - 1);
	mxz = clamp(mxz, 0, size - 1);

	// Track entire updated region.
	if (m_applied)
	{
		m_dirtyRegion[0] = min(m_dirtyRegion[0], mnx);
		m_dirtyRegion[1] = min(m_dirtyRegion[1], mnz);
		m_dirtyRegion[2] = max(m_dirtyRegion[2], mxx);
		m_dirtyRegion[3] = max(m_dirtyRegion[3], mxz);
	}
	else
	{
		m_dirtyRegion[0] = mnx;
		m_dirtyRegion[1] = mnz;
		m_dirtyRegion[2] = mxx;
		m_dirtyRegion[3] = mxz;
	}

	const int32_t region[] = { mnx, mnz, mxx, mxz };
	updateTiles(region, m_brushMode);

	m_context->raiseRedraw(nullptr);
	m_applied = true;
}

void TerrainEditModifier::updateTiles(const int32_t* region, uint32_t brushMode)
{
	const Terrain* terrain = m_terrainComponent->getTerrain();
	const int32_t tileCount = terrain->getTileCount();
	const int32_t tileSize = terrain->getTileSize();
	if (tileCount != m_data->getTileCount() || tileSize != m_data->getTileSize() || (int32_t)m_terrainComponent->m_tiles.size() != tileCount * tileCount)
		return;

	m_tileMaps.resize(tileCount * tileCount);

	// Tiles also contain first row and column of next tile.
	const int32_t mnx = clamp((region[0] - 1) / tileSize, 0, tileCount - 1);
	const int32_t mnz = clamp((region[1] - 1) / tileSize, 0, tileCount - 1);
	const int32_t mxx = clamp(region[2] / tileSize, 0, tileCount - 1);
	const int32_t mxz = clamp(region[3] / tileSize, 0, tileCount - 1);
	const int32_t mapSize = tileSize + 1;

	for (int32_t tz = mnz; tz <= mxz; ++tz)
	{
		for (int32_t tx = mnx; tx <= mxx; ++tx)
		{
			const int32_t tileIndex = tx + tz * tileCount;
			const int32_t x0 = tx * tileSize;
			const int32_t z0 = tz * tileSize;
			TileMaps& maps = m_tileMaps[tileIndex];

			// Tile maps are created, from edited data, first time tile is edited.
			const bool create = (maps.heightMap == nullptr);
			if (create)
			{
				maps.heightMap = createTileMap(m_context->getRenderSystem(), mapSize, render::TfR32F);
				maps.normalMap = createTileMap(m_context->getRenderSystem(), mapSize, render::TfR8G8B8A8);
				maps.splatMap = createTileMap(m_context->getRenderSystem(), mapSize, render::TfR8G8B8A8);
				maps.colorMap = createTileMap(m_context->getRenderSystem(), mapSize, render::TfR8G8B8A8);
				maps.cutMap = createTileMap(m_context->getRenderSystem(), mapSize, render::TfR8);
				if (!maps.heightMap || !maps.normalMap || !maps.splatMap || !maps.colorMap || !maps.cutMap)
				{
					maps = TileMaps();
					continue;
				}
			}

			const int32_t tileRegion[] = {
				create ? 0 : clamp(region[0] - x0, 0, mapSize - 1),
				create ? 0 : clamp(region[1] - z0, 0, mapSize - 1),
				create ? mapSize - 1 : clamp(region[2] - x0, 0, mapSize - 1),
				create ? mapSize - 1 : clamp(region[3] - z0, 0, mapSize - 1)
			};

			if (create || (brushMode & IBrush::MdHeight) != 0)
			{
				fillTileMap(maps.heightMap, TileMap::Heights, m_data, x0, z0, tileRegion);
				fillTileMap(maps.normalMap, TileMap::Normals, m_data, x0, z0, tileRegion);
			}
			if (create || (brushMode & IBrush::MdMaterial) != 0)
				fillTileMap(maps.splatMap, TileMap::Splat, m_data, x0, z0, tileRegion);
			if (create || (brushMode & IBrush::MdColor) != 0)
				fillTileMap(maps.colorMap, TileMap::Color, m_data, x0, z0, tileRegion);
			if (create || (brushMode & IBrush::MdCut) != 0)
				fillTileMap(maps.cutMap, TileMap::Cuts, m_data, x0, z0, tileRegion);

			// Tile maps are assigned again if tiles has been recreated, such as when terrain is reloaded.
			if (m_terrainComponent->m_tiles[tileIndex].resources.heightMap.getResource() != maps.heightMap)
				m_terrainComponent->editTile(tileIndex, maps.heightMap, maps.normalMap, maps.splatMap, maps.colorMap, maps.cutMap);

			// Tile's heightfield is used for picking, patch bounds and terrain layers.
			hf::Heightfield* heightfield = m_terrainComponent->m_tiles[tileIndex].resources.heightfield;
			if (heightfield && (brushMode & (IBrush::MdHeight | IBrush::MdCut | IBrush::MdAttribute)) != 0)
			{
				for (int32_t z = tileRegion[1]; z <= tileRegion[3]; ++z)
				{
					for (int32_t x = tileRegion[0]; x <= tileRegion[2]; ++x)
					{
						heightfield->setGridHeight(x, z, m_data->getGridHeightNearest(x0 + x, z0 + z));
						heightfield->setGridCut(x, z, m_data->getGridCut(x0 + x, z0 + z));
						if ((brushMode & IBrush::MdAttribute) != 0)
							heightfield->setGridAttribute(x, z, (uint8_t)m_attribute, m_data->getGridAttributeDensity(x0 + x, z0 + z, (uint8_t)m_attribute));
					}
				}
				heightfield->updateCellBounds(tileRegion[0], tileRegion[1], tileRegion[2], tileRegion[3]);
			}
		}
	}
}

void TerrainEditModifier::end()
{
	// Terrain layers are scattered from tile heightfields.
	for (auto terrainLayer : m_terrainLayers)
		terrainLayer->updatePatches();

	float gx, gz;
	m_data->worldToGrid(m_center.x(), m_center.z(), gx, gz);
	m_brush->end(gx, gz);

	// Update error metrics of edited tiles after stroke to prevent popping while drawing.
	if (
		(m_brushMode & IBrush::MdHeight) != 0 &&
		m_applied
	)
	{
		const int32_t tileCount = m_data->getTileCount();
		const int32_t tileSize = m_data->getTileSize();

		for (int32_t i = 0; i < (int32_t)m_tileMaps.size(); ++i)
		{
			const int32_t x0 = (i % tileCount) * tileSize;
			const int32_t z0 = (i / tileCount) * tileSize;
			if (
				!m_tileMaps[i].heightMap ||
				m_dirtyRegion[2] < x0 || m_dirtyRegion[0] > x0 + tileSize ||
				m_dirtyRegion[3] < z0 || m_dirtyRegion[1] > z0 + tileSize
			)
				continue;

			m_terrainComponent->updateTilePatches(i, m_dirtyRegion);
		}
	}

	// Write modified tiles of splat and color.
	if (
		(m_brushMode & (IBrush::MdMaterial | IBrush::MdColor)) != 0 &&
		m_terrainInstance
	)
	{
		if (!m_context->getDocument()->containInstance(m_terrainInstance))
		{
			if (!m_terrainInstance->checkout())
			{
				log::error << L"Unable to checkout terrain instance." << Endl;
				return;
			}
		}

		if (!m_data->writeImages(m_terrainInstance))
			log::error << L"Unable to write terrain images." << Endl;

		m_context->getDocument()->editInstance(
			m_terrainInstance,
			nullptr
		);
		m_context->getDocument()->setModified();
	}

	// Write modified tiles of heightfield.
	if (
		(m_brushMode & (IBrush::MdHeight | IBrush::MdCut | IBrush::MdAttribute)) != 0 &&
		m_heightfieldInstance
	)
	{
		if (!m_context->getDocument()->containInstance(m_heightfieldInstance))
		{
			if (!m_heightfieldInstance->checkout())
			{
				log::error << L"Unable to checkout heightfield instance." << Endl;
				return;
			}

			m_heightfieldAsset = m_heightfieldInstance->getObject< hf::HeightfieldAsset >();
			if (!m_heightfieldAsset)
			{
				m_heightfieldInstance->revert();
				return;
			}

			m_context->getDocument()->editInstance(
				m_heightfieldInstance,
				m_heightfieldAsset
			);
		}

		if (m_data->writeHeightfield(m_heightfieldInstance))
			m_context->getDocument()->setModified();
		else
			log::error << L"Unable to write heights." << Endl;
	}

	m_context->raiseRedraw(nullptr);
}

void TerrainEditModifier::flattenUnderSpline()
{
	Ref< IBrush > currentBrush = m_brush;

	RefArray< scene::EntityAdapter > entityAdapters;
	m_context->findAdaptersOfType(
		type_of< shape::SplineComponent >(),
		entityAdapters,
		scene::SceneEditorContext::GfDefault | scene::SceneEditorContext::GfSelectedOnly | scene::SceneEditorContext::GfNoExternalChild
	);

	FlattenBrush flattenBrush(m_data);
	m_brush = &flattenBrush;

	for (auto entityAdapter : entityAdapters)
	{
		auto splineComponent = entityAdapter->getComponent< shape::SplineComponent >();
		T_FATAL_ASSERT(splineComponent != nullptr);

		const auto& path = splineComponent->getPath();
		const auto& keys = path.keys();
		if (keys.empty())
			return;

		if (!begin(false))
			continue;

	 	const float st = path.getStartTime();
	 	const float et = path.getEndTime();

		const uint32_t nsteps = keys.size() * 20;
		for (uint32_t i = 0; i <= nsteps; ++i)
		{
			const float t = st + (float)(i * (et - st)) / nsteps;
			auto v = path.evaluate(t, true);
			const Vector4 center = v.position.xyz1();
			flattenBrush.setHeight(center.y());
			apply(center);
		}

		end();
	}

	m_brush = currentBrush;
}

	}
}
