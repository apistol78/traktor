#include "Core/RefArray.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldFormat.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/Editor/AverageBrush.h"
#include "Terrain/Editor/ElevateBrush.h"
#include "Terrain/Editor/FlattenBrush.h"
#include "Terrain/Editor/SharpFallOff.h"
#include "Terrain/Editor/SmoothBrush.h"
#include "Terrain/Editor/SmoothFallOff.h"
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
,	m_center(Vector4::zero())
,	m_strength(1.0f)
{
}

void TerrainEditModifier::selectionChanged()
{
	m_heightfieldInstance = 0;
	m_heightfieldAsset = 0;
	m_heightfield.clear();
	m_brush = 0;

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

	// Create normal texture data.
	int32_t size = m_heightfield->getSize();
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
	render::SimpleTextureCreateDesc desc;
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

	// Create default brush.
	m_brush = new ElevateBrush(m_heightfield);
	m_fallOff = new SmoothFallOff();
}

bool TerrainEditModifier::cursorMoved(
	const scene::TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection
)
{
	if (!m_entity)
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
	if (!m_heightfield)
		return false;

	if (command == L"Terrain.Editor.AverageBrush")
		m_brush = new AverageBrush(m_heightfield);
	else if (command == L"Terrain.Editor.ElevateBrush")
		m_brush = new ElevateBrush(m_heightfield);
	else if (command == L"Terrain.Editor.FlattenBrush")
		m_brush = new FlattenBrush(m_heightfield);
	else if (command == L"Terrain.Editor.SmoothBrush")
		m_brush = new SmoothBrush(m_heightfield);
	if (command == L"Terrain.Editor.SmoothFallOff")
		m_fallOff = new SmoothFallOff();
	else if (command == L"Terrain.Editor.SharpFallOff")
		m_fallOff = new SharpFallOff();
	else
		return false;

	return true;
}

bool TerrainEditModifier::begin(
	const scene::TransformChain& transformChain,
	int32_t mouseButton
)
{
	float worldRadius = m_context->getGuideSize();
	int32_t gridRadius = int32_t(m_heightfield->getSize() * worldRadius / m_heightfield->getWorldExtent().x());

	int32_t gx, gz;
	m_heightfield->worldToGrid(m_center.x(), m_center.z(), gx, gz);
	m_brush->begin(
		gx,
		gz,
		gridRadius,
		m_fallOff,
		m_strength * (mouseButton == 1 ? 1.0f : -1.0f)
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
	visitor.brush = m_brush;
	line(gx0, gz0, gx1, gz1, visitor);

	m_entity->updatePatches();
	m_center = center;

	// Update normals.
	{
		float worldRadius = m_context->getGuideSize();
		int32_t gridRadius = int32_t(m_heightfield->getSize() * worldRadius / m_heightfield->getWorldExtent().x());

		int32_t size = m_heightfield->getSize();

		int32_t mnx = min(gx0 - gridRadius, gx1 - gridRadius), mxx = max(gx0 + gridRadius, gx1 + gridRadius);
		int32_t mnz = min(gz0 - gridRadius, gz1 - gridRadius), mxz = max(gz0 + gridRadius, gz1 + gridRadius);

		mnx = clamp(mnx, 0, size - 1);
		mxx = clamp(mxx, 0, size - 1);
		mnz = clamp(mnz, 0, size - 1);
		mxz = clamp(mxz, 0, size - 1);

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
}

void TerrainEditModifier::end(const scene::TransformChain& transformChain)
{
	db::Database* sourceDatabase = m_context->getEditor()->getSourceDatabase();
	T_ASSERT (sourceDatabase);

	int32_t gx, gz;
	m_heightfield->worldToGrid(m_center.x(), m_center.z(), gx, gz);
	m_brush->end(gx, gz);

	// Write modifications to heightfield.
	if (!m_heightfieldInstance)
	{
		const resource::Id< Terrain >& terrain = m_entityData->getTerrain();
		Ref< const TerrainAsset > terrainAsset = sourceDatabase->getObjectReadOnly< TerrainAsset >(terrain);
		if (!terrainAsset)
			return;

		const resource::Id< hf::Heightfield >& heightfield = terrainAsset->getHeightfield();
		m_heightfieldInstance = sourceDatabase->getInstance(heightfield);
		if (!m_heightfieldInstance)
			return;

		if (!m_heightfieldInstance->checkout())
		{
			m_heightfieldInstance = 0;
			return;
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

	if (m_heightfieldInstance)
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
	if (!m_entity || m_center.w() <= FUZZY_EPSILON)
		return;

	float radius = m_context->getGuideSize();

	primitiveRenderer->drawSolidPoint(m_center, 8, Color4ub(255, 0, 0, 255));
	primitiveRenderer->pushDepthEnable(false);

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

	primitiveRenderer->popDepthEnable();
}

	}
}
