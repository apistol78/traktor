/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <limits>
#include "Core/RefArray.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
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
#include "Terrain/TerrainUtilities.h"
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

	render::SimpleTextureCreateDesc desc;

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

	// Get runtime heightfield.
	m_heightfield = m_terrainComponent->getTerrain()->getHeightfield();
	if (!m_heightfield)
	{
		m_terrainComponent = nullptr;
		m_terrainComponentData = nullptr;
		return false;
	}

	const int32_t size = m_heightfield->getSize();

	m_terrainInstance = sourceDatabase->getInstance(m_terrainComponentData->getTerrain());

	// Create non-compressed texture for heights.
	desc.width = size;
	desc.height = size;
	desc.mipCount = 1;
	desc.format = render::TfR32F;
	desc.sRGB = false;
	desc.immutable = false;

	m_heightMap = m_context->getRenderSystem()->createSimpleTexture(desc, T_FILE_LINE_W);
	if (m_heightMap)
	{
		// Transfer heights to texture.
		render::ITexture::Lock nl;
		if (m_heightMap->lock(0, 0, nl))
		{
			float* ptr = (float*)nl.bits;
			for (int32_t v = 0; v < size; ++v)
			{
				for (int32_t u = 0; u < size; ++u)
				{
					const float height = m_heightfield->getGridHeightNearest(u, v); // * asset->m_scale;
					*ptr++ = height;
				}
			}
			m_heightMap->unlock(0, 0);
		}

		// Replace height map in resource with our texture.
		m_terrainComponent->m_terrain->m_heightMap = resource::Proxy< render::ITexture >(m_heightMap);
	}

	// Get splat image from terrain asset.
	if (m_terrainInstance)
	{
		Ref< IStream > file = m_terrainInstance->readData(L"Splat");
		if (file)
		{
			m_splatImage = drawing::Image::load(file, L"tri");
			if (!m_splatImage)
			{
				file->seek(IStream::SeekSet, 0);
				m_splatImage = drawing::Image::load(file, L"tga");
			}
			if (m_splatImage)
				m_splatImage->convert(drawing::PixelFormat::getR8G8B8A8().endianSwapped());
			file->close();
			file = nullptr;
		}
	}

	// Create splat image if none attached.
	if (!m_splatImage)
	{
		m_splatImage = new drawing::Image(drawing::PixelFormat::getR8G8B8A8().endianSwapped(), size, size);
		m_splatImage->clear(Color4f(1.0f, 0.0f, 0.0f, 0.0f));
	}

	// Ensure splat image is of correct size.
	if (m_splatImage->getWidth() != size)
	{
		drawing::ScaleFilter scaleFilter(size, size, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
		m_splatImage->apply(&scaleFilter);
	}

	// Create non-compressed texture for splats.
	desc.width = size;
	desc.height = size;
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.sRGB = false;
	desc.immutable = false;

	m_splatMap = m_context->getRenderSystem()->createSimpleTexture(desc, T_FILE_LINE_W);
	if (m_splatMap)
	{
		// Transfer splats to texture.
		render::ITexture::Lock nl;
		if (m_splatMap->lock(0, 0, nl))
		{
			std::memcpy(nl.bits, m_splatImage->getData(), size * size * 4);
			m_splatMap->unlock(0, 0);
		}

		// Replace splat map in resource with our texture.
		m_terrainComponent->m_terrain->m_splatMap = resource::Proxy< render::ITexture >(m_splatMap);
	}

	// Get color image from terrain asset.
	if (m_terrainInstance)
	{
		Ref< IStream > file = m_terrainInstance->readData(L"Color");
		if (file)
		{
			m_colorImage = drawing::Image::load(file, L"tri");
			if (!m_colorImage)
			{
				file->seek(IStream::SeekSet, 0);
				m_colorImage = drawing::Image::load(file, L"tga");
			}
			if (m_colorImage)
				m_colorImage->convert(drawing::PixelFormat::getABGRF32());
			file->close();
			file = nullptr;
		}
	}

	// Create color image if none attached.
	if (!m_colorImage)
	{
		m_colorImage = new drawing::Image(drawing::PixelFormat::getABGRF32(), size, size);
		m_colorImage->clear(Color4f(0.5f, 0.5f, 0.5f, 0.0f));
	}

	// Ensure color image is of correct size.
	if (m_colorImage->getWidth() != size)
	{
		drawing::ScaleFilter scaleFilter(size, size, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
		m_colorImage->apply(&scaleFilter);
	}

	// Create low-precision color image used for transfer.
	m_colorImageLowPrecision = new drawing::Image(drawing::PixelFormat::getA8B8G8R8(), size, size);
	m_colorImageLowPrecision->copy(m_colorImage, 0, 0, size, size);

	// Create non-compressed texture for colors.
	desc.width = size;
	desc.height = size;
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.sRGB = false;
	desc.immutable = false;

	m_colorMap = m_context->getRenderSystem()->createSimpleTexture(desc, T_FILE_LINE_W);
	if (m_colorMap)
	{
		// Transfer colors to texture.
		render::ITexture::Lock nl;
		if (m_colorMap->lock(0, 0, nl))
		{
			std::memcpy(nl.bits, m_colorImageLowPrecision->getData(), size * size * 4);
			m_colorMap->unlock(0, 0);
		}

		// Replace color map in resource with our texture.
		m_terrainComponent->m_terrain->m_colorMap = resource::Proxy< render::ITexture >(m_colorMap);
	}

	// Create normal texture data.
	m_normalData.reset(new uint8_t [size * size * 4]);
	for (int32_t v = 0; v < size; ++v)
	{
		for (int32_t u = 0; u < size; ++u)
		{
			const Vector4 normal = m_heightfield->normalAt((float)u, (float)v) * Vector4(0.5f, 0.5f, 0.5f, 0.0f) + Vector4(0.5f, 0.5f, 0.5f, 0.0f);
			const uint8_t nx = uint8_t(normal.x() * 255);
			const uint8_t ny = uint8_t(normal.y() * 255);
			const uint8_t nz = uint8_t(normal.z() * 255);

			uint8_t* ptr = &m_normalData[(u + v * size) * 4];
			ptr[0] = nx;
			ptr[1] = nz;
			ptr[2] = ny;
			ptr[3] = 0;
		}
	}

	// Create non-compressed texture for normals.
	desc.width = size;
	desc.height = size;
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.sRGB = false;
	desc.immutable = false;

	m_normalMap = m_context->getRenderSystem()->createSimpleTexture(desc, T_FILE_LINE_W);
	if (m_normalMap)
	{
		// Transfer normals to texture.
		render::ITexture::Lock nl;
		if (m_normalMap->lock(0, 0, nl))
		{
			std::memcpy(nl.bits, m_normalData.c_ptr(), size * size * 4);
			m_normalMap->unlock(0, 0);
		}

		// Replace normal map in resource with our texture.
		m_terrainComponent->m_terrain->m_normalMap = resource::Proxy< render::ITexture >(m_normalMap);
	}

	// Create cut texture data.
	m_cutData.reset(new uint8_t [size * size]);
	for (int32_t v = 0; v < size; ++v)
	{
		for (int32_t u = 0; u < size; ++u)
		{
			m_cutData[u + v * size] = m_heightfield->getGridCut(u, v) ? 0x00 : 0xff;
		}
	}

	// Create non-compressed texture for cut data.
	desc.width = size;
	desc.height = size;
	desc.mipCount = 1;
	desc.format = render::TfR8;
	desc.sRGB = false;
	desc.immutable = false;

	m_cutMap = m_context->getRenderSystem()->createSimpleTexture(desc, T_FILE_LINE_W);
	if (m_cutMap)
	{
		// Transfer cuts to texture.
		render::ITexture::Lock nl;
		if (m_cutMap->lock(0, 0, nl))
		{
			std::memcpy(nl.bits, m_cutData.c_ptr(), size * size);
			m_cutMap->unlock(0, 0);
		}

		// Replace cut map in resource with our texture.
		m_terrainComponent->m_terrain->m_cutMap = resource::Proxy< render::ITexture >(m_cutMap);
	}

	// Create material mask texture data.
	m_attributeData.reset(new uint8_t [size * size]);
	for (int32_t v = 0; v < size; ++v)
	{
		for (int32_t u = 0; u < size; ++u)
		{
			m_attributeData[u + v * size] = m_heightfield->getGridAttribute(u, v);
		}
	}

	// Create default brush; try set same brush type as before.
	if (m_brush)
		setBrush(type_of(m_brush));

	if (!m_brush)
		m_brush = new ElevateBrush(m_heightfield, m_splatImage);

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
	m_heightfield.clear();
	m_splatImage = nullptr;
	m_colorImage = nullptr;
	m_colorImageLowPrecision = nullptr;
	m_colorMap = nullptr;
	m_normalMap = nullptr;
	m_cutMap = nullptr;
}

void TerrainEditModifier::selectionChanged()
{
}

scene::IModifier::CursorMovedResult TerrainEditModifier::cursorMoved(
	const scene::TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection
)
{
	if (!m_terrainComponent || !m_heightfield)
		return { false, false };

	const Vector4 lastCenter = m_center;
	bool hot = false;

	Scalar distance;
	if (m_heightfield->queryRay(
		worldRayOrigin,
		worldRayDirection,
		distance
	))
	{
		m_center = (worldRayOrigin + worldRayDirection * distance).xyz1();
		hot = true;
	}
	else
		m_center = Vector4::zero();

	return { hot, m_center != lastCenter };
}

bool TerrainEditModifier::handleCommand(const ui::Command& command)
{
	if (command == L"Terrain.Editor.FlattenUnderSpline")
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
	if (!m_terrainComponent || m_center.w() <= FUZZY_EPSILON)
		return;

	// Find furthest intersection which allows for editing around hills etc.
	Vector4 center = worldRayOrigin;
	bool found = false;
	for (;;)
	{
		center += worldRayDirection * 0.01_simd;

		Scalar distance;
		if (!m_heightfield->queryRay(
			center,
			worldRayDirection,
			distance
		))
			break;

		center = (center + worldRayDirection * distance).xyz1();
		found = true;
	}
	if (!found)
		return;

	apply(center);
}

void TerrainEditModifier::end(const scene::TransformChain& transformChain)
{
	end();
}

void TerrainEditModifier::draw(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (!m_terrainComponent || !m_heightfield || m_center.w() <= FUZZY_EPSILON)
		return;

	const float radius = m_context->getGuideSize();

	primitiveRenderer->drawSolidPoint(m_center, 8, Color4ub(255, 0, 0, 255));
	primitiveRenderer->pushDepthState(false, false, false);

	float x0 = m_center.x() + cosf(0.0f) * radius;
	float z0 = m_center.z() + sinf(0.0f) * radius;
	float y0 = m_heightfield->getWorldHeight(x0, z0);

	for (int32_t i = 1; i <= 32; ++i)
	{
		const float a = TWO_PI * i / 32.0f;

		const float x1 = m_center.x() + cosf(a) * radius;
		const float z1 = m_center.z() + sinf(a) * radius;
		const float y1 = m_heightfield->getWorldHeight(x1, z1);

		primitiveRenderer->drawLine(
			Vector4(x0, y0 + FUZZY_EPSILON, z0, 1.0f),
			Vector4(x1, y1 + FUZZY_EPSILON, z1, 1.0f),
			1.0f,
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
		m_brush = new ColorBrush(m_colorImage);
	else if (is_type_a< CutBrush >(brushType))
		m_brush = new CutBrush(m_heightfield);
	else if (is_type_a< ElevateBrush >(brushType))
		m_brush = new ElevateBrush(m_heightfield, m_splatImage);
	else if (is_type_a< FlattenBrush >(brushType))
		m_brush = new FlattenBrush(m_heightfield);
	else if (is_type_a< MaterialBrush >(brushType))
		m_brush = new MaterialBrush(m_heightfield, m_splatImage);
	else if (is_type_a< NoiseBrush >(brushType))
		m_brush = new NoiseBrush(m_heightfield);
	else if (is_type_a< SmoothBrush >(brushType))
		m_brush = new SmoothBrush(m_heightfield);
	else if (is_type_a< AttributeBrush >(brushType))
		m_brush = new AttributeBrush(m_heightfield);
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
	if (!m_heightfield)
		return false;

	m_context->setPlaying(false);

	float worldRadius = m_context->getGuideSize();
	int32_t gridRadius = int32_t(m_heightfield->getSize() * worldRadius / m_heightfield->getWorldExtent().x());

	float gx, gz;
	m_heightfield->worldToGrid(m_center.x(), m_center.z(), gx, gz);

	IBrush::State state;
	state.radius = gridRadius;
	state.falloff = m_fallOff;
	state.strength = m_strength * (inverted ? -1.0f : 1.0f);
	state.color = m_color;
	state.material = m_material;
	state.attribute = m_attribute;

	m_brushMode = m_brush->begin(gx, gz, state);

	m_updateRegion[0] =
	m_updateRegion[1] =
	m_updateRegion[2] =
	m_updateRegion[3] = 0;
	m_applied = false;
	return true;
}

void TerrainEditModifier::apply(const Vector4& center)
{
	Terrain* terrain = m_terrainComponent->getTerrain();
	if (!terrain)
		return;

	if (!m_applied)
		m_center = center;

	float gx0, gz0;
	float gx1, gz1;	

	m_heightfield->worldToGrid(m_center.x(), m_center.z(), gx0, gz0);
	m_heightfield->worldToGrid(center.x(), center.z(), gx1, gz1);

	// Apply brush along entire line.
	line_dda(gx0, gz0, gx1, gz1, [&](int32_t x, int32_t y) {
		m_brush->apply(x, y);
	});

	m_center = center;

	const int32_t size = m_heightfield->getSize();

	// Calculate region which needs to be updated.
	const float worldRadius = m_context->getGuideSize();
	const int32_t gridRadius = int32_t(size * worldRadius / m_heightfield->getWorldExtent().x());

	int32_t mnx = (int32_t)min(std::floor(gx0) - gridRadius, std::floor(gx1) - gridRadius);
	int32_t mxx = (int32_t)max(std::ceil(gx0) + gridRadius, std::ceil(gx1) + gridRadius);
	int32_t mnz = (int32_t)min(std::floor(gz0) - gridRadius, std::floor(gz1) - gridRadius);
	int32_t mxz = (int32_t)max(std::ceil(gz0) + gridRadius, std::ceil(gz1) + gridRadius);

	mnx = clamp(mnx, 0, size - 1);
	mxx = clamp(mxx, 0, size - 1);
	mnz = clamp(mnz, 0, size - 1);
	mxz = clamp(mxz, 0, size - 1);

	const uint32_t pmnx = gridToPatch(m_heightfield, terrain->getPatchDim(), terrain->getDetailSkip(), mnx);
	const uint32_t pmxx = gridToPatch(m_heightfield, terrain->getPatchDim(), terrain->getDetailSkip(), mxx);
	const uint32_t pmnz = gridToPatch(m_heightfield, terrain->getPatchDim(), terrain->getDetailSkip(), mnz);
	const uint32_t pmxz = gridToPatch(m_heightfield, terrain->getPatchDim(), terrain->getDetailSkip(), mxz);

	const uint32_t region[] = { pmnx, pmnz, pmxx, pmxz };
	m_terrainComponent->updatePatches(region, false, true);

	// Track entire updated region.
	if (m_applied)
	{
		m_updateRegion[0] = min(m_updateRegion[0], region[0]);
		m_updateRegion[1] = min(m_updateRegion[1], region[1]);
		m_updateRegion[2] = max(m_updateRegion[2], region[2]);
		m_updateRegion[3] = max(m_updateRegion[3], region[3]);
	}
	else
	{
		for (int32_t i = 0; i < 4; ++i)
			m_updateRegion[i] = region[i];
	}

	RefArray< Job > jobs;

	// Update heights.
	if ((m_brushMode & IBrush::MdHeight) != 0)
	{
		jobs.push_back(JobManager::getInstance().add([&](){
			// Transfer heights to texture.
			render::ITexture::Lock nl;
			if (m_heightMap->lock(0, 0, nl))
			{
				hf::Heightfield* hf = m_heightfield;
				float* ptr = (float*)nl.bits;
				for (int32_t v = 0; v < size; ++v)
				{
					for (int32_t u = 0; u < size; ++u)
					{
						const float height = hf->getGridHeightNearest(u, v);
						*ptr++ = height;
					}
				}
				m_heightMap->unlock(0, 0);
			}

			// Replace height map in resource with our texture.
			m_terrainComponent->m_terrain->m_heightMap = resource::Proxy< render::ITexture >(m_heightMap);
		}));
	}

	// Update splats.
	if ((m_brushMode & IBrush::MdMaterial) != 0)
	{
		jobs.push_back(JobManager::getInstance().add([&](){
			// Transfer splats to texture.
			render::ITexture::Lock cl;
			if (m_splatMap->lock(0, 0, cl))
			{
				const uint8_t* src = static_cast< const uint8_t* >(m_splatImage->getData());
				uint8_t* dst = static_cast< uint8_t* >(cl.bits);

				for (int32_t y = 0; y < size; ++y)
					std::memcpy(&dst[y * cl.pitch], &src[y * size * 4], size * 4);

				m_splatMap->unlock(0, 0);
			}

			// Replace splat map in resource with our texture.
			m_terrainComponent->m_terrain->m_splatMap = resource::Proxy< render::ITexture >(m_splatMap);
		}));
	}

	// Update colors.
	if ((m_brushMode & IBrush::MdColor) != 0)
	{
		jobs.push_back(JobManager::getInstance().add([&](){
			// Transfer colors to texture.
			render::ITexture::Lock cl;
			if (m_colorMap->lock(0, 0, cl))
			{
				m_colorImageLowPrecision->copy(m_colorImage, mnx, mnz, mnx, mnz, mxx - mnx, mxz - mnz);

				const uint8_t* src = static_cast< const uint8_t* >(m_colorImageLowPrecision->getData());
				uint8_t* dst = static_cast< uint8_t* >(cl.bits);

				for (int32_t y = 0; y < size; ++y)
					std::memcpy(&dst[y * cl.pitch], &src[y * size * 4], size * 4);

				m_colorMap->unlock(0, 0);
			}

			// Replace color map in resource with our texture.
			m_terrainComponent->m_terrain->m_colorMap = resource::Proxy< render::ITexture >(m_colorMap);
		}));
	}

	// Update normals.
	if ((m_brushMode & IBrush::MdHeight) != 0)
	{
		jobs.push_back(JobManager::getInstance().add([&](){
			hf::Heightfield* hf = m_heightfield;

			for (int32_t v = mnz; v <= mxz; ++v)
			{
				for (int32_t u = mnx; u <= mxx; ++u)
				{
					const Vector4 normal = hf->normalAt((float)u, (float)v) * Vector4(0.5f, 0.5f, 0.5f, 0.0f) + Vector4(0.5f, 0.5f, 0.5f, 0.0f);
					const uint8_t nx = uint8_t(normal.x() * 255);
					const uint8_t ny = uint8_t(normal.y() * 255);
					const uint8_t nz = uint8_t(normal.z() * 255);
					uint8_t* ptr = &m_normalData[(u + v * size) * 4];
					ptr[0] = nx;
					ptr[1] = nz;
					ptr[2] = ny;
					ptr[3] = 0;
				}
			}

			// Transfer normals to texture.
			render::ITexture::Lock nl;
			if (m_normalMap->lock(0, 0, nl))
			{
				const uint8_t* src = static_cast< const uint8_t* >(m_normalData.c_ptr());
				uint8_t* dst = static_cast< uint8_t* >(nl.bits);

				for (int32_t y = 0; y < size; ++y)
					std::memcpy(&dst[y * nl.pitch], &src[y * size * 4], size * 4);

				m_normalMap->unlock(0, 0);
			}

			// Replace normal map in resource with our texture.
			m_terrainComponent->m_terrain->m_normalMap = resource::Proxy< render::ITexture >(m_normalMap);
		}));
	}

	// Update cuts.
	if ((m_brushMode & IBrush::MdCut) != 0)
	{
		jobs.push_back(JobManager::getInstance().add([&](){
			hf::Heightfield* hf = m_heightfield;

			for (int32_t v = mnz; v <= mxz; ++v)
			{
				for (int32_t u = mnx; u <= mxx; ++u)
				{
					m_cutData[u + v * size] = hf->getGridCut(u, v) ? 0x00 : 0xff;
				}
			}

			// Transfer cuts to texture.
			render::ITexture::Lock cl;
			if (m_cutMap->lock(0, 0, cl))
			{
				const uint8_t* src = static_cast< const uint8_t* >(m_cutData.c_ptr());
				uint8_t* dst = static_cast< uint8_t* >(cl.bits);

				for (int32_t y = 0; y < size; ++y)
					std::memcpy(&dst[y * cl.pitch], &src[y * size], size);

				m_cutMap->unlock(0, 0);
			}

			// Replace cut map in resource with our texture.
			m_terrainComponent->m_terrain->m_cutMap = resource::Proxy< render::ITexture >(m_cutMap);
		}));
	}

	// Update material mask.
	if ((m_brushMode & IBrush::MdAttribute) != 0)
	{
		hf::Heightfield* hf = m_heightfield;
		for (int32_t v = mnz; v <= mxz; ++v)
		{
			for (int32_t u = mnx; u <= mxx; ++u)
			{
				m_attributeData[u + v * size] = hf->getGridAttribute(u, v);
			}
		}
	}

	// Wait until all transfers has finished.
	while (!jobs.empty())
	{
		jobs.back()->wait();
		jobs.pop_back();
	}

	m_context->raiseRedraw(nullptr);
	m_applied = true;	
}

void TerrainEditModifier::end()
{
	db::Database* sourceDatabase = m_context->getEditor()->getSourceDatabase();
	T_ASSERT(sourceDatabase);

	// Update errors metrics after modifier to prevent popping while drawing.
	m_terrainComponent->updatePatches(nullptr, true, false);
	for (auto terrainLayer : m_terrainLayers)
		terrainLayer->updatePatches();

	float gx, gz;
	m_heightfield->worldToGrid(m_center.x(), m_center.z(), gx, gz);
	m_brush->end(gx, gz);

	// Update patch lod metrics if heights has been modified.
	if (
		(m_brushMode & IBrush::MdHeight) != 0 &&
		m_applied
	)
	{
		Terrain* terrain = m_terrainComponent->getTerrain();
		T_ASSERT(terrain);

		auto& patches = terrain->editPatches();

		const uint32_t heightfieldSize = m_heightfield->getSize();
		const uint32_t patchCount = heightfieldSize / (terrain->getPatchDim() * terrain->getDetailSkip());

		for (uint32_t pz = m_updateRegion[1]; pz <= m_updateRegion[3]; ++pz)
		{
			for (uint32_t px = m_updateRegion[0]; px <= m_updateRegion[2]; ++px)
			{
				Terrain::Patch& patch = patches[px + pz * patchCount];
				calculatePatchMinMaxHeight(m_heightfield, px, pz, terrain->getPatchDim(), terrain->getDetailSkip(), patch.height);
				calculatePatchErrorMetrics(m_heightfield, 4, px, pz, terrain->getPatchDim(), terrain->getDetailSkip(), patch.error);
			}
		}
	}

	// Write modifications to splats.
	if (
		(m_brushMode & IBrush::MdMaterial) != 0 &&
		m_terrainInstance
	)
	{
		if (!m_context->getDocument()->containInstance(m_terrainInstance))
		{
			if (!m_terrainInstance->checkout())
			{
				m_terrainInstance = nullptr;
				return;
			}
		}

		Ref< IStream > file = m_terrainInstance->writeData(L"Splat");
		if (file)
		{
			m_splatImage->save(file, L"tri");
			file->close();
			file = nullptr;
		}

		m_context->getDocument()->editInstance(
			m_terrainInstance,
			nullptr
		);
		m_context->getDocument()->setModified();
	}

	// Write modifications to color.
	if (
		(m_brushMode & IBrush::MdColor) != 0 &&
		m_terrainInstance
	)
	{
		if (!m_context->getDocument()->containInstance(m_terrainInstance))
		{
			if (!m_terrainInstance->checkout())
			{
				m_terrainInstance = nullptr;
				return;
			}
		}

		Ref< IStream > file = m_terrainInstance->writeData(L"Color");
		if (file)
		{
			m_colorImageLowPrecision->save(file, L"tri");
			file->close();
			file = nullptr;
		}

		m_context->getDocument()->editInstance(
			m_terrainInstance,
			nullptr
		);
		m_context->getDocument()->setModified();
	}

	// Checkout heightfield asset.
	if (
		(m_brushMode & (IBrush::MdHeight | IBrush::MdCut | IBrush::MdAttribute)) != 0 &&
		!m_heightfieldInstance
	)
	{
		const resource::Id< Terrain >& terrain = m_terrainComponentData->getTerrain();
		Ref< const TerrainAsset > terrainAsset = sourceDatabase->getObjectReadOnly< TerrainAsset >(terrain);
		if (!terrainAsset)
			return;

		const resource::Id< hf::Heightfield >& heightfield = terrainAsset->getHeightfield();
		m_heightfieldInstance = sourceDatabase->getInstance(heightfield);
		if (!m_heightfieldInstance)
			return;

		if (!m_context->getDocument()->containInstance(m_heightfieldInstance))
		{
			if (!m_heightfieldInstance->checkout())
			{
				m_heightfieldInstance = nullptr;
				return;
			}
		}

		m_heightfieldAsset = m_heightfieldInstance->getObject< hf::HeightfieldAsset >();
		if (!m_heightfieldAsset)
		{
			m_heightfieldInstance->revert();
			m_heightfieldInstance = nullptr;
			return;
		}

		m_context->getDocument()->editInstance(
			m_heightfieldInstance,
			m_heightfieldAsset
		);
	}

	// Write modifications to heightfield.
	if (
		(m_brushMode & (IBrush::MdHeight | IBrush::MdCut | IBrush::MdAttribute)) != 0 &&
		m_heightfieldInstance
	)
	{
		Ref< IStream > data = m_heightfieldInstance->writeData(L"Data");
		if (data)
		{
			hf::HeightfieldFormat().write(data, m_heightfield);

			data->close();
			data = nullptr;

			m_context->getDocument()->setModified();
		}
		else
			log::error << L"Unable to write heights" << Endl;
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

	FlattenBrush flattenBrush(m_heightfield);
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
