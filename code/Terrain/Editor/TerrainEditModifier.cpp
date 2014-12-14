#include <cstring>
#include "Core/RefArray.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
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
#include "Terrain/TerrainEntity.h"
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
void line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, Visitor& visitor)
{
	float dx = float(x1 - x0);
	float dy = float(y1 - y0);

	visitor(x0, y0);

	if (std::abs(dx) > std::abs(dy) && dx != 0)
	{
		if (dx < 0)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		float k = dy / dx;
		float y = y0 + k;

		for (int32_t x = x0 + 1; x < x1; ++x)
		{
			visitor(x, int32_t(y + 0.5f));
			y += k;
		}
	}
	else if (dy != 0)
	{
		if (dy < 0)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		float k = dx / dy;
		float x = x0 + k;

		for (int32_t y = y0 + 1; y < y1; ++y)
		{
			visitor(int32_t(x + 0.5f), y);
			x += k;
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
,	m_color(Color4f(1.0f, 1.0f, 1.0f, 1.0f))
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

	// Get terrain entity from selection.
	RefArray< scene::EntityAdapter > entityAdapters;
	if (m_context->getEntities(entityAdapters, scene::SceneEditorContext::GfDefault | scene::SceneEditorContext::GfSelectedOnly | scene::SceneEditorContext::GfNoExternalChild) <= 0)
		return;

	m_entity = dynamic_type_cast< TerrainEntity* >(entityAdapters[0]->getEntity());
	m_entityData = dynamic_type_cast< TerrainEntityData* >(entityAdapters[0]->getEntityData());

	// Ensure we've both entity and it's data.
	if (!m_entity || !m_entityData)
	{
		m_entity = 0;
		m_entityData = 0;
		return;
	}

	// Get runtime heightfield.
	m_heightfield = m_entity->getTerrain()->getHeightfield();
	if (!m_heightfield)
	{
		m_entity = 0;
		m_entityData = 0;
		return;
	}

	int32_t size = m_heightfield->getSize();

	m_terrainInstance = sourceDatabase->getInstance(m_entityData->getTerrain());

	// Get splat image from terrain asset.
	if (m_terrainInstance)
	{
		Ref< IStream > file = m_terrainInstance->readData(L"Splat");
		if (file)
		{
			m_splatImage = drawing::Image::load(file, L"tga");
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
		m_entity->m_terrain->m_splatMap = resource::Proxy< render::ISimpleTexture >(m_splatMap);
	}

	// Get color image from terrain asset.
	if (m_terrainInstance)
	{
		Ref< IStream > file = m_terrainInstance->readData(L"Color");
		if (file)
		{
			m_colorImage = drawing::Image::load(file, L"tga");
			m_colorImage->convert(drawing::PixelFormat::getR8G8B8A8().endianSwapped());
			file->close();
			file = 0;
		}
	}

	// Create color image if none attached.
	if (!m_colorImage)
	{
		m_colorImage = new drawing::Image(drawing::PixelFormat::getR8G8B8A8().endianSwapped(), size, size);
		m_colorImage->clear(Color4f(0.5f, 0.5f, 0.5f, 0.0f));
	}

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
			std::memcpy(nl.bits, m_colorImage->getData(), size * size * 4);
			m_colorMap->unlock(0);
		}

		// Replace color map in resource with our texture.
		m_entity->m_terrain->m_colorMap = resource::Proxy< render::ISimpleTexture >(m_colorMap);
	}

	// Create normal texture data.
	m_normalData.reset(new uint8_t [size * size * 4]);
	for (int32_t v = 0; v < size; ++v)
	{
		for (int32_t u = 0; u < size; ++u)
		{
			Vector4 normal = normalAt(m_heightfield, u, v) * Vector4(0.5f, 0.5f, 0.5f, 0.0f) + Vector4(0.5f, 0.5f, 0.5f, 0.0f);
			uint8_t nx = uint8_t(255 - normal.x() * 255);
			uint8_t ny = uint8_t(normal.y() * 255);
			uint8_t* ptr = &m_normalData[(u + v * size) * 4];
			ptr[0] = 0;
			ptr[1] = ny;
			ptr[2] = 0;
			ptr[3] = nx;
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
		m_entity->m_terrain->m_normalMap = resource::Proxy< render::ISimpleTexture >(m_normalMap);
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
		// Transfer normals to texture.
		render::ITexture::Lock nl;
		if (m_cutMap->lock(0, nl))
		{
			std::memcpy(nl.bits, m_cutData.c_ptr(), size * size);
			m_cutMap->unlock(0);
		}

		// Replace normal map in resource with our texture.
		m_entity->m_terrain->m_cutMap = resource::Proxy< render::ISimpleTexture >(m_cutMap);
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
	m_entity->setVisualizeMode(m_visualizeMode);
}

bool TerrainEditModifier::cursorMoved(
	const scene::TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection
)
{
	if (!m_entity || !m_heightfield)
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
	if (!m_entity || m_center.w() <= FUZZY_EPSILON)
		return;

	Scalar distance;
	if (!m_heightfield->queryRay(
		worldRayOrigin,
		worldRayDirection,
		distance
	))
		return;

	Vector4 center = (worldRayOrigin + worldRayDirection * distance).xyz1();

	int32_t gx0, gz0;
	int32_t gx1, gz1;

	m_heightfield->worldToGrid(m_center.x(), m_center.z(), gx0, gz0);
	m_heightfield->worldToGrid(center.x(), center.z(), gx1, gz1);

	BrushVisitor visitor;
	visitor.brush = m_spatialBrush;
	line(gx0, gz0, gx1, gz1, visitor);

	m_entity->updatePatches();
	m_center = center;

	int32_t size = m_heightfield->getSize();
	int32_t mnx = 0, mxx = size - 1;
	int32_t mnz = 0, mxz = size - 1;

	// Calculate region which needs to be updated; only
	// applies to "contained" brushes.
	if (m_spatialBrush->contained())
	{
		float worldRadius = m_context->getGuideSize();
		int32_t gridRadius = int32_t(size * worldRadius / m_heightfield->getWorldExtent().x());

		mnx = min(gx0 - gridRadius, gx1 - gridRadius), mxx = max(gx0 + gridRadius, gx1 + gridRadius);
		mnz = min(gz0 - gridRadius, gz1 - gridRadius), mxz = max(gz0 + gridRadius, gz1 + gridRadius);

		mnx = clamp(mnx, 0, size - 1);
		mxx = clamp(mxx, 0, size - 1);
		mnz = clamp(mnz, 0, size - 1);
		mxz = clamp(mxz, 0, size - 1);
	}

	// Update splats.
	if ((m_brushMode & IBrush::MdSplat) != 0)
	{
		// Transfer splats to texture.
		render::ITexture::Lock cl;
		if (m_splatMap->lock(0, cl))
		{
			std::memcpy(cl.bits, m_splatImage->getData(), size * size * 4);
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
			std::memcpy(cl.bits, m_colorImage->getData(), size * size * 4);
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
				uint8_t nx = uint8_t(255 - normal.x() * 255);
				uint8_t ny = uint8_t(normal.y() * 255);
				uint8_t* ptr = &m_normalData[(u + v * size) * 4];
				ptr[0] = 0;
				ptr[1] = ny;
				ptr[2] = 0;
				ptr[3] = nx;
			}
		}

		// Transfer normals to texture.
		render::ITexture::Lock nl;
		if (m_normalMap->lock(0, nl))
		{
			std::memcpy(nl.bits, m_normalData.c_ptr(), size * size * 4);
			m_normalMap->unlock(0);
		}
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
			std::memcpy(cl.bits, m_cutData.c_ptr(), size * size);
			m_cutMap->unlock(0);
		}
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
			m_splatImage->save(file, L"tga");
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
			m_colorImage->save(file, L"tga");
			file->close();
			file = 0;
		}

		m_context->getDocument()->editInstance(
			m_terrainInstance,
			0
		);
		m_context->getDocument()->setModified();
	}

	// Write modifications to heightfield.
	if (
		(m_brushMode & (IBrush::MdHeight | IBrush::MdCut)) != 0 &&
		!m_heightfieldInstance
	)
	{
		const resource::Id< Terrain >& terrain = m_entityData->getTerrain();
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

	if (
		(m_brushMode & (IBrush::MdHeight | IBrush::MdCut)) != 0 &&
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
	if (!m_entity || !m_heightfield || m_center.w() <= FUZZY_EPSILON)
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
	else if (is_type_a< MaterialBrush >(brushType))
		m_drawBrush = new MaterialBrush(m_splatImage);
	else if (is_type_a< NoiseBrush >(brushType))
		m_drawBrush = new NoiseBrush(m_heightfield);
	else if (is_type_a< ErodeBrush >(brushType))
		m_drawBrush = new ErodeBrush(m_heightfield);
	else if (is_type_a< SmoothBrush >(brushType))
		m_drawBrush = new SmoothBrush(m_heightfield);

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
}

void TerrainEditModifier::setMaterial(int32_t material)
{
	m_material = material;
}

void TerrainEditModifier::setVisualizeMode(TerrainEntity::VisualizeMode visualizeMode)
{
	m_visualizeMode = visualizeMode;
	if (m_entity)
		m_entity->setVisualizeMode(m_visualizeMode);
}

void TerrainEditModifier::setFallOffImage(drawing::Image* fallOffImage)
{
	m_fallOffImage = fallOffImage;
}

	}
}
