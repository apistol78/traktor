/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include <limits>
#include "Core/RefArray.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
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
#include "Render/ISimpleTexture.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainComponentData.h"
#include "Terrain/TerrainUtilities.h"
#include "Terrain/Editor/AverageBrush.h"
#include "Terrain/Editor/ColorBrush.h"
#include "Terrain/Editor/CutBrush.h"
#include "Terrain/Editor/ElevateBrush.h"
#include "Terrain/Editor/EmissiveBrush.h"
#include "Terrain/Editor/ErodeBrush.h"
#include "Terrain/Editor/FlattenBrush.h"
#include "Terrain/Editor/ImageFallOff.h"
#include "Terrain/Editor/MaterialBrush.h"
#include "Terrain/Editor/NoiseBrush.h"
#include "Terrain/Editor/SharpFallOff.h"
#include "Terrain/Editor/SmoothBrush.h"
#include "Terrain/Editor/SmoothFallOff.h"
#include "Terrain/Editor/SplatBrush.h"
#include "Terrain/Editor/SymmetricalBrush.h"
#include "Terrain/Editor/TerrainAsset.h"
#include "Terrain/Editor/TerrainEditModifier.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

Vector4 normalAt(const hf::Heightfield* heightfield, int32_t u, int32_t v)
{
	const float c_distance = 0.5f;
	const float directions[][2] =
	{
		{ -c_distance, -c_distance },
		{        0.0f, -c_distance },
		{  c_distance, -c_distance },
		{  c_distance,        0.0f },
		{  c_distance,  c_distance },
		{        0.0f,        0.0f },
		{ -c_distance,  c_distance },
		{ -c_distance,        0.0f }
	};

	float h0 = heightfield->getGridHeightNearest(u, v);

	float h[sizeof_array(directions)];
	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
		h[i] = heightfield->getGridHeightBilinear(u + directions[i][0], v + directions[i][1]);

	const Vector4& worldExtent = heightfield->getWorldExtent();
	float sx = worldExtent.x() / heightfield->getSize();
	float sy = worldExtent.y();
	float sz = worldExtent.z() / heightfield->getSize();

	Vector4 N = Vector4::zero();

	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
	{
		uint32_t j = (i + 1) % sizeof_array(directions);

		float dx1 = directions[i][0] * sx;
		float dy1 = (h[i] - h0) * sy;
		float dz1 = directions[i][1] * sz;

		float dx2 = directions[j][0] * sx;
		float dy2 = (h[j] - h0) * sy;
		float dz2 = directions[j][1] * sz;

		Vector4 n = cross(
			Vector4(dx2, dy2, dz2),
			Vector4(dx1, dy1, dz1)
		);

		N += n;
	}

	return N.normalized();
}

template < typename Visitor >
void line_dda(float x0, float y0, float x1, float y1, Visitor& visitor)
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
		tmaxx = std::numeric_limits< float >::max();

	if (std::abs(dy) > FUZZY_EPSILON)
	{
		ryr = 1.0f / dy;
		tmaxy = (cby - y0) * ryr;
		tdeltay = stepy * ryr;
	}
	else
		tmaxy = std::numeric_limits< float >::max();

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

struct BrushVisitor
{
	IBrush* brush;

	void operator () (int32_t x, int32_t y)
	{
		brush->apply(x, y);
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEditModifier", TerrainEditModifier, scene::IModifier)

TerrainEditModifier::TerrainEditModifier(scene::SceneEditorContext* context)
:	m_context(context)
,	m_brushMode(0)
,	m_symmetry(0)
,	m_strength(1.0f)
,	m_color(Color4f(0.5f, 0.5f, 0.5f, 1.0f))
,	m_material(0)
,	m_center(Vector4::zero())
{
}

void TerrainEditModifier::selectionChanged()
{
	db::Database* sourceDatabase = m_context->getEditor()->getSourceDatabase();
	T_ASSERT (sourceDatabase);

	render::SimpleTextureCreateDesc desc;

	m_terrainInstance = 0;
	m_heightfieldInstance = 0;
	m_heightfieldAsset = 0;
	m_heightfield.clear();
	m_splatImage = 0;
	m_colorImage = 0;

	// Get terrain component from selection.
	RefArray< scene::EntityAdapter > entityAdapters;
	if (m_context->getEntities(entityAdapters, scene::SceneEditorContext::GfDefault | scene::SceneEditorContext::GfSelectedOnly | scene::SceneEditorContext::GfNoExternalChild) <= 0)
		return;

	m_terrainComponent = entityAdapters[0]->getComponent< TerrainComponent >();
	m_terrainComponentData = entityAdapters[0]->getComponentData< TerrainComponentData >();

	// Ensure we've both component and it's data.
	if (!m_terrainComponent || !m_terrainComponentData)
	{
		m_terrainComponent = 0;
		m_terrainComponentData = 0;
		return;
	}

	// Get runtime heightfield.
	m_heightfield = m_terrainComponent->getTerrain()->getHeightfield();
	if (!m_heightfield)
	{
		m_terrainComponent = 0;
		m_terrainComponentData = 0;
		return;
	}

	int32_t size = m_heightfield->getSize();

	m_terrainInstance = sourceDatabase->getInstance(m_terrainComponentData->getTerrain());

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
			m_splatImage->convert(drawing::PixelFormat::getR8G8B8A8().endianSwapped());
			file->close();
			file = 0;
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

	m_splatMap = m_context->getRenderSystem()->createSimpleTexture(desc);
	if (m_splatMap)
	{
		// Transfer splats to texture.
		render::ITexture::Lock nl;
		if (m_splatMap->lock(0, nl))
		{
			std::memcpy(nl.bits, m_splatImage->getData(), size * size * 4);
			m_splatMap->unlock(0);
		}

		// Replace splat map in resource with our texture.
		m_terrainComponent->m_terrain->m_splatMap = resource::Proxy< render::ISimpleTexture >(m_splatMap);
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
			m_colorImage->convert(drawing::PixelFormat::getABGRF32());
			file->close();
			file = 0;
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

	m_colorMap = m_context->getRenderSystem()->createSimpleTexture(desc);
	if (m_colorMap)
	{
		// Transfer colors to texture.
		render::ITexture::Lock nl;
		if (m_colorMap->lock(0, nl))
		{
			std::memcpy(nl.bits, m_colorImageLowPrecision->getData(), size * size * 4);
			m_colorMap->unlock(0);
		}

		// Replace color map in resource with our texture.
		m_terrainComponent->m_terrain->m_colorMap = resource::Proxy< render::ISimpleTexture >(m_colorMap);
	}

	// Create normal texture data.
	m_normalData.reset(new uint8_t [size * size * 4]);
	for (int32_t v = 0; v < size; ++v)
	{
		for (int32_t u = 0; u < size; ++u)
		{
			Vector4 normal = normalAt(m_heightfield, u, v) * Vector4(0.5f, 0.5f, 0.5f, 0.0f) + Vector4(0.5f, 0.5f, 0.5f, 0.0f);
			uint8_t nx = uint8_t(normal.x() * 255);
			uint8_t ny = uint8_t(normal.y() * 255);
			uint8_t nz = uint8_t(normal.z() * 255);
			uint8_t* ptr = &m_normalData[(u + v * size) * 4];
			ptr[0] = nx;
			ptr[1] = ny;
			ptr[2] = nz;
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

	m_normalMap = m_context->getRenderSystem()->createSimpleTexture(desc);
	if (m_normalMap)
	{
		// Transfer normals to texture.
		render::ITexture::Lock nl;
		if (m_normalMap->lock(0, nl))
		{
			std::memcpy(nl.bits, m_normalData.c_ptr(), size * size * 4);
			m_normalMap->unlock(0);
		}

		// Replace normal map in resource with our texture.
		m_terrainComponent->m_terrain->m_normalMap = resource::Proxy< render::ISimpleTexture >(m_normalMap);
	}

	// Create cut texture data.
	m_cutData.reset(new uint8_t [size * size]);
	for (int32_t v = 0; v < size; ++v)
	{
		for (int32_t u = 0; u < size; ++u)
		{
			m_cutData[u + v * size] = m_heightfield->getGridCut(u, v) ? 0xff : 0x00;
		}
	}

	// Create non-compressed texture for cut data.
	desc.width = size;
	desc.height = size;
	desc.mipCount = 1;
	desc.format = render::TfR8;
	desc.sRGB = false;
	desc.immutable = false;

	m_cutMap = m_context->getRenderSystem()->createSimpleTexture(desc);
	if (m_cutMap)
	{
		// Transfer cuts to texture.
		render::ITexture::Lock nl;
		if (m_cutMap->lock(0, nl))
		{
			std::memcpy(nl.bits, m_cutData.c_ptr(), size * size);
			m_cutMap->unlock(0);
		}

		// Replace cut map in resource with our texture.
		m_terrainComponent->m_terrain->m_cutMap = resource::Proxy< render::ISimpleTexture >(m_cutMap);
	}

	// Create material mask texture data.
	m_materialData.reset(new uint8_t [size * size]);
	for (int32_t v = 0; v < size; ++v)
	{
		for (int32_t u = 0; u < size; ++u)
		{
			m_materialData[u + v * size] = m_heightfield->getGridMaterial(u, v);
		}
	}

	// Create non-compressed texture for cut data.
	desc.width = size;
	desc.height = size;
	desc.mipCount = 1;
	desc.format = render::TfR8;
	desc.sRGB = false;
	desc.immutable = false;

	m_materialMap = m_context->getRenderSystem()->createSimpleTexture(desc);
	if (m_materialMap)
	{
		// Transfer material mask to texture.
		render::ITexture::Lock nl;
		if (m_materialMap->lock(0, nl))
		{
			std::memcpy(nl.bits, m_materialData.c_ptr(), size * size);
			m_materialMap->unlock(0);
		}

		// Replace material mask map in resource with our texture.
		m_terrainComponent->m_terrain->m_materialMap = resource::Proxy< render::ISimpleTexture >(m_materialMap);
	}

	// Create default brush; try set same brush type as before.
	if (m_drawBrush)
		setBrush(type_of(m_drawBrush));

	if (!m_drawBrush)
		m_drawBrush = new ElevateBrush(m_heightfield);

	T_ASSERT(m_drawBrush);
	m_spatialBrush = m_drawBrush;

	if (!m_fallOff)
		m_fallOff = new SmoothFallOff();

	// Set visualize mode.
	m_terrainComponent->setVisualizeMode(m_visualizeMode);
}

bool TerrainEditModifier::cursorMoved(
	const scene::TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection
)
{
	if (!m_terrainComponent || !m_heightfield)
		return false;

	Scalar distance;
	if (m_heightfield->queryRay(
		worldRayOrigin,
		worldRayDirection,
		distance
	))
		m_center = (worldRayOrigin + worldRayDirection * distance).xyz1();
	else
		m_center = Vector4::zero();

	return true;
}

bool TerrainEditModifier::handleCommand(const ui::Command& command)
{
	return false;
}

bool TerrainEditModifier::begin(
	const scene::TransformChain& transformChain,
	int32_t mouseButton
)
{
	if (!m_heightfield)
		return false;

	m_context->setPlaying(false);

	float worldRadius = m_context->getGuideSize();
	int32_t gridRadius = int32_t(m_heightfield->getSize() * worldRadius / m_heightfield->getWorldExtent().x());

	int32_t gx, gz;
	m_heightfield->worldToGrid(m_center.x(), m_center.z(), gx, gz);

	m_brushMode = m_spatialBrush->begin(
		gx,
		gz,
		gridRadius,
		m_fallOff,
		m_strength * (mouseButton == 1 ? 1.0f : -1.0f),
		m_color,
		m_material
	);

	return true;
}

void TerrainEditModifier::apply(
	const scene::TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	const Vector4& screenDelta,
	const Vector4& viewDelta
)
{
	if (!m_terrainComponent || m_center.w() <= FUZZY_EPSILON)
		return;

	Scalar distance;
	if (!m_heightfield->queryRay(
		worldRayOrigin,
		worldRayDirection,
		distance
	))
		return;

	Vector4 center = (worldRayOrigin + worldRayDirection * distance).xyz1();

	float gx0, gz0;
	float gx1, gz1;

	m_heightfield->worldToGrid(m_center.x(), m_center.z(), gx0, gz0);
	m_heightfield->worldToGrid(center.x(), center.z(), gx1, gz1);

	BrushVisitor visitor;
	visitor.brush = m_spatialBrush;
	line_dda(gx0, gz0, gx1, gz1, visitor);

	m_terrainComponent->updatePatches();
	m_center = center;

	int32_t size = m_heightfield->getSize();

	// Calculate region which needs to be updated; only
	// applies to "contained" brushes.
	int32_t mnx = 0, mxx = size - 1;
	int32_t mnz = 0, mxz = size - 1;

	if (m_spatialBrush->contained())
	{
		float worldRadius = m_context->getGuideSize();
		int32_t gridRadius = int32_t(size * worldRadius / m_heightfield->getWorldExtent().x());

		mnx = min(std::floor(gx0) - gridRadius, std::floor(gx1) - gridRadius), mxx = max(std::ceil(gx0) + gridRadius, std::ceil(gx1) + gridRadius);
		mnz = min(std::floor(gz0) - gridRadius, std::floor(gz1) - gridRadius), mxz = max(std::ceil(gz0) + gridRadius, std::ceil(gz1) + gridRadius);

		mnx = clamp(mnx, 0, size - 1);
		mxx = clamp(mxx, 0, size - 1);
		mnz = clamp(mnz, 0, size - 1);
		mxz = clamp(mxz, 0, size - 1);
	}

	// Update patch lod metrics if heights has been modified.
	if ((m_brushMode & IBrush::MdHeight) != 0)
	{
		Terrain* terrain = m_terrainComponent->getTerrain();
		if (terrain)
		{
			std::vector< Terrain::Patch >& patches = terrain->editPatches();

			uint32_t heightfieldSize = m_heightfield->getSize();
			uint32_t patchCount = heightfieldSize / (terrain->getPatchDim() * terrain->getDetailSkip());

			uint32_t pmnx = gridToPatch(m_heightfield, terrain->getPatchDim(), terrain->getDetailSkip(), mnx);
			uint32_t pmxx = gridToPatch(m_heightfield, terrain->getPatchDim(), terrain->getDetailSkip(), mxx);
			uint32_t pmnz = gridToPatch(m_heightfield, terrain->getPatchDim(), terrain->getDetailSkip(), mnz);
			uint32_t pmxz = gridToPatch(m_heightfield, terrain->getPatchDim(), terrain->getDetailSkip(), mxz);

			for (uint32_t pz = pmnz; pz <= pmxz; ++pz)
			{
				for (uint32_t px = pmnx; px <= pmxx; ++px)
				{
					Terrain::Patch& patch = patches[px + pz * patchCount];
					calculatePatchMinMaxHeight(m_heightfield, px, pz, terrain->getPatchDim(), terrain->getDetailSkip(), patch.height);
					calculatePatchErrorMetrics(m_heightfield, 4, px, pz, terrain->getPatchDim(), terrain->getDetailSkip(), patch.error);
				}
			}
		}
	}

	// Update splats.
	if ((m_brushMode & IBrush::MdSplat) != 0)
	{
		// Transfer splats to texture.
		render::ITexture::Lock cl;
		if (m_splatMap->lock(0, cl))
		{
			const uint8_t* src = static_cast< const uint8_t* >(m_splatImage->getData());
			uint8_t* dst = static_cast< uint8_t* >(cl.bits);

			for (int32_t y = 0; y < size; ++y)
				std::memcpy(&dst[y * cl.pitch], &src[y * size * 4], size * 4);

			m_splatMap->unlock(0);
		}
	}

	// Update colors.
	if ((m_brushMode & IBrush::MdColor) != 0)
	{
		// Transfer colors to texture.
		render::ITexture::Lock cl;
		if (m_colorMap->lock(0, cl))
		{
			m_colorImageLowPrecision->copy(m_colorImage, mnx, mnz, mnx, mnz, mxx - mnx, mxz - mnz);
			
			const uint8_t* src = static_cast< const uint8_t* >(m_colorImageLowPrecision->getData());
			uint8_t* dst = static_cast< uint8_t* >(cl.bits);

			for (int32_t y = 0; y < size; ++y)
				std::memcpy(&dst[y * cl.pitch], &src[y * size * 4], size * 4);

			m_colorMap->unlock(0);
		}
	}

	// Update normals.
	if ((m_brushMode & IBrush::MdHeight) != 0)
	{
		for (int32_t v = mnz; v <= mxz; ++v)
		{
			for (int32_t u = mnx; u <= mxx; ++u)
			{
				Vector4 normal = normalAt(m_heightfield, u, v) * Vector4(0.5f, 0.5f, 0.5f, 0.0f) + Vector4(0.5f, 0.5f, 0.5f, 0.0f);
				uint8_t nx = uint8_t(normal.x() * 255);
				uint8_t ny = uint8_t(normal.y() * 255);
				uint8_t nz = uint8_t(normal.z() * 255);
				uint8_t* ptr = &m_normalData[(u + v * size) * 4];
				ptr[0] = nx;
				ptr[1] = ny;
				ptr[2] = nz;
				ptr[3] = 0;
			}
		}

		// Transfer normals to texture.
		render::ITexture::Lock nl;
		if (m_normalMap->lock(0, nl))
		{
			const uint8_t* src = static_cast< const uint8_t* >(m_normalData.c_ptr());
			uint8_t* dst = static_cast< uint8_t* >(nl.bits);

			for (int32_t y = 0; y < size; ++y)
				std::memcpy(&dst[y * nl.pitch], &src[y * size * 4], size * 4);

			m_normalMap->unlock(0);
		}

		// Replace normal map in resource with our texture.
		m_terrainComponent->m_terrain->m_normalMap = resource::Proxy< render::ISimpleTexture >(m_normalMap);
	}

	// Update cuts.
	if ((m_brushMode & IBrush::MdCut) != 0)
	{
		for (int32_t v = mnz; v <= mxz; ++v)
		{
			for (int32_t u = mnx; u <= mxx; ++u)
			{
				m_cutData[u + v * size] = m_heightfield->getGridCut(u, v) ? 0xff : 0x00;
			}
		}

		// Transfer cuts to texture.
		render::ITexture::Lock cl;
		if (m_cutMap->lock(0, cl))
		{
			const uint8_t* src = static_cast< const uint8_t* >(m_cutData.c_ptr());
			uint8_t* dst = static_cast< uint8_t* >(cl.bits);

			for (int32_t y = 0; y < size; ++y)
				std::memcpy(&dst[y * cl.pitch], &src[y * size], size);

			m_cutMap->unlock(0);
		}

		// Replace cut map in resource with our texture.
		m_terrainComponent->m_terrain->m_cutMap = resource::Proxy< render::ISimpleTexture >(m_cutMap);
	}

	// Update material mask.
	if ((m_brushMode & IBrush::MdMaterial) != 0)
	{
		for (int32_t v = mnz; v <= mxz; ++v)
		{
			for (int32_t u = mnx; u <= mxx; ++u)
			{
				m_materialData[u + v * size] = m_heightfield->getGridMaterial(u, v);
			}
		}

		// Transfer cuts to texture.
		render::ITexture::Lock cl;
		if (m_materialMap->lock(0, cl))
		{
			const uint8_t* src = static_cast< const uint8_t* >(m_materialData.c_ptr());
			uint8_t* dst = static_cast< uint8_t* >(cl.bits);

			for (int32_t y = 0; y < size; ++y)
				std::memcpy(&dst[y * cl.pitch], &src[y * size], size);

			m_materialMap->unlock(0);
		}

		// Replace material mask map in resource with our texture.
		m_terrainComponent->m_terrain->m_materialMap = resource::Proxy< render::ISimpleTexture >(m_materialMap);
	}
}

void TerrainEditModifier::end(const scene::TransformChain& transformChain)
{
	db::Database* sourceDatabase = m_context->getEditor()->getSourceDatabase();
	T_ASSERT (sourceDatabase);

	int32_t gx, gz;
	m_heightfield->worldToGrid(m_center.x(), m_center.z(), gx, gz);
	m_spatialBrush->end(gx, gz);

	// Write modifications to splats.
	if (
		(m_brushMode & IBrush::MdSplat) != 0 &&
		m_terrainInstance
	)
	{
		if (!m_context->getDocument()->containInstance(m_terrainInstance))
		{
			if (!m_terrainInstance->checkout())
			{
				m_terrainInstance = 0;
				return;
			}
		}

		Ref< IStream > file = m_terrainInstance->writeData(L"Splat");
		if (file)
		{
			m_splatImage->save(file, L"tri");
			file->close();
			file = 0;
		}

		m_context->getDocument()->editInstance(
			m_terrainInstance,
			0
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
				m_terrainInstance = 0;
				return;
			}
		}

		Ref< IStream > file = m_terrainInstance->writeData(L"Color");
		if (file)
		{
			m_colorImageLowPrecision->save(file, L"tri");
			file->close();
			file = 0;
		}

		m_context->getDocument()->editInstance(
			m_terrainInstance,
			0
		);
		m_context->getDocument()->setModified();
	}

	// Checkout heightfield asset.
	if (
		(m_brushMode & (IBrush::MdHeight | IBrush::MdCut | IBrush::MdMaterial)) != 0 &&
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
				m_heightfieldInstance = 0;
				return;
			}
		}

		m_heightfieldAsset = m_heightfieldInstance->getObject< hf::HeightfieldAsset >();
		if (!m_heightfieldAsset)
		{
			m_heightfieldInstance->revert();
			m_heightfieldInstance = 0;
			return;
		}

		m_context->getDocument()->editInstance(
			m_heightfieldInstance,
			m_heightfieldAsset
		);
	}

	// Write modifications to heightfield.
	if (
		(m_brushMode & (IBrush::MdHeight | IBrush::MdCut | IBrush::MdMaterial)) != 0 &&
		m_heightfieldInstance
	)
	{
		Ref< IStream > data = m_heightfieldInstance->writeData(L"Data");
		if (data)
		{
			hf::HeightfieldFormat().write(data, m_heightfield);

			data->close();
			data = 0;

			m_context->getDocument()->setModified();
		}
		else
			log::error << L"Unable to write heights" << Endl;
	}
}

void TerrainEditModifier::draw(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (!m_terrainComponent || !m_heightfield || m_center.w() <= FUZZY_EPSILON)
		return;

	float radius = m_context->getGuideSize();

	primitiveRenderer->drawSolidPoint(m_center, 8, Color4ub(255, 0, 0, 255));
	primitiveRenderer->pushDepthState(false, false, false);

	float a0 = 0.0f;
	float x0 = m_center.x() + cosf(a0) * radius;
	float z0 = m_center.z() + sinf(a0) * radius;
	float y0 = m_heightfield->getWorldHeight(x0, z0);

	for (int32_t i = 1; i <= 32; ++i)
	{
		float a1 = TWO_PI * i / 32.0f;
		float x1 = m_center.x() + cosf(a1) * radius;
		float z1 = m_center.z() + sinf(a1) * radius;
		float y1 = m_heightfield->getWorldHeight(x1, z1);

		primitiveRenderer->drawLine(
			Vector4(x0, y0 + FUZZY_EPSILON, z0, 1.0f),
			Vector4(x1, y1 + FUZZY_EPSILON, z1, 1.0f),
			1.0f,
			Color4ub(255, 0, 0, 180)
		);

		a0 = a1;
		x0 = x1;
		z0 = z1;
		y0 = y1;
	}

	primitiveRenderer->popDepthState();
}

void TerrainEditModifier::setBrush(const TypeInfo& brushType)
{
	if (is_type_a< AverageBrush >(brushType))
		m_drawBrush = new AverageBrush(m_heightfield);
	else if (is_type_a< ColorBrush >(brushType))
		m_drawBrush = new ColorBrush(m_colorImage);
	else if (is_type_a< EmissiveBrush >(brushType))
		m_drawBrush = new EmissiveBrush(m_colorImage);
	else if (is_type_a< CutBrush >(brushType))
		m_drawBrush = new CutBrush(m_heightfield);
	else if (is_type_a< ElevateBrush >(brushType))
		m_drawBrush = new ElevateBrush(m_heightfield);
	else if (is_type_a< FlattenBrush >(brushType))
		m_drawBrush = new FlattenBrush(m_heightfield);
	else if (is_type_a< SplatBrush >(brushType))
		m_drawBrush = new SplatBrush(m_splatImage);
	else if (is_type_a< NoiseBrush >(brushType))
		m_drawBrush = new NoiseBrush(m_heightfield);
	else if (is_type_a< ErodeBrush >(brushType))
		m_drawBrush = new ErodeBrush(m_heightfield);
	else if (is_type_a< SmoothBrush >(brushType))
		m_drawBrush = new SmoothBrush(m_heightfield);
	else if (is_type_a< MaterialBrush >(brushType))
		m_drawBrush = new MaterialBrush(m_heightfield);

	m_spatialBrush = m_drawBrush;
	if (m_symmetry & 1)
	{
		int32_t scale[] = { -1, 1 };
		int32_t offset[] = { m_heightfield->getSize(), 0 };
		m_spatialBrush = new SymmetricalBrush(scale, offset, m_spatialBrush);
	}
	if (m_symmetry & 2)
	{
		int32_t scale[] = { 1, -1 };
		int32_t offset[] = { 0, m_heightfield->getSize() };
		m_spatialBrush = new SymmetricalBrush(scale, offset, m_spatialBrush);
	}
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

void TerrainEditModifier::setSymmetry(uint32_t symmetry)
{
	m_symmetry = symmetry;
	m_spatialBrush = m_drawBrush;

	int32_t scale[] = { 1, 1 };
	int32_t offset[] = { 0, 0 };

	if (m_symmetry & 1)
	{
		scale[0] = -1;
		offset[0] = m_heightfield->getSize();
	}
	if (m_symmetry & 2)
	{
		scale[1] = -1;
		offset[1] = m_heightfield->getSize();
	}

	if (m_symmetry != 0)
		m_spatialBrush = new SymmetricalBrush(scale, offset, m_spatialBrush);
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

	}
}
