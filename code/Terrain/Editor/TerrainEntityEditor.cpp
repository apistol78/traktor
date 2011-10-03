#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Editor/IEditor.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/FlattenBrush.h"
#include "Heightfield/Editor/HeightfieldCompositor.h"
#include "Heightfield/Editor/HeightfieldEditorPlugin.h"
#include "Heightfield/Editor/RoundBrush.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/Editor/TerrainEntityEditor.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEntityEditor", TerrainEntityEditor, scene::DefaultEntityEditor)

Ref< TerrainEntityEditor > TerrainEntityEditor::create(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
{
	Ref< TerrainEntity > terrainEntity = checked_type_cast< TerrainEntity* >(entityAdapter->getEntity());

	hf::HeightfieldEditorPlugin* heightfieldPlugin = context->getEditorPluginOf< hf::HeightfieldEditorPlugin >();
	if (!heightfieldPlugin)
		return 0;

	hf::HeightfieldCompositor* compositor = heightfieldPlugin->getCompositor(terrainEntity->getHeightfield().getGuid());
	if (!compositor)
		return 0;

	return new TerrainEntityEditor(context, entityAdapter, compositor);
}

bool TerrainEntityEditor::queryRay(
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	Scalar& outDistance
) const
{
	return m_compositor->queryRay(worldRayOrigin, worldRayDirection, outDistance);
}

void TerrainEntityEditor::cursorMoved(const ApplyParams& params)
{
	Scalar distance;
	m_compositor->queryRay(params.worldRayOrigin, params.worldRayDirection, distance, &m_strokeLast);
}

void TerrainEntityEditor::beginModifier(const ApplyParams& params)
{
	Scalar distance;
	if (m_compositor->queryRay(params.worldRayOrigin, params.worldRayDirection, distance, &m_strokeLast))
	{
		if (m_brushType == 0)
			m_brush = new hf::RoundBrush(m_brushRadius, params.mouseButton == 0 ? 1.0f : -1.0f);
		if (m_brushType == 1)
			m_brush = new hf::FlattenBrush(m_brushRadius);

		DefaultEntityEditor::beginModifier(params);
	}
}

void TerrainEntityEditor::applyModifier(const ApplyParams& params)
{
	if (!inModify())
		return;

	Vector4 position;
	Scalar distance;

	if (m_compositor->queryRay(params.worldRayOrigin, params.worldRayDirection, distance, &position))
	{
		if (params.mouseButton == 2)
		{
			m_brushRadius = clamp< float >((position - m_strokeLast).length(), 0.1f, 10.0f);
		}
		else
		{
			hf::Region dirtyRegion;
			m_compositor->strokeBrush(m_strokeLast, position, m_brush, &dirtyRegion);

			if (!dirtyRegion.empty())
				updateHeightfield(true, false, false, dirtyRegion);

			m_strokeLast = position;
		}
	}
}

void TerrainEntityEditor::endModifier(const ApplyParams& params)
{
	if (!inModify())
		return;

	if (params.mouseButton != 2)
	{
		hf::Region dirtyRegion;
		m_compositor->end(&dirtyRegion);

		if (!dirtyRegion.empty())
			updateHeightfield(true, true, true, dirtyRegion);
	}

	DefaultEntityEditor::endModifier(params);
}

bool TerrainEntityEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Heightfield.Raise")
		m_brushType = 0;
	if (command == L"Heightfield.Lower")
		m_brushType = 1;

	return false;
}

void TerrainEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	const int32_t c_segments = 30;
	const float c_offset = 0.4f;

	Color4ub brushColor = inModify() ? Color4ub(255, 255, 0, 255) : Color4ub(128, 255, 128, 255);

	float cx = m_strokeLast.x();
	float cz = m_strokeLast.z();

	for (int32_t i = 0; i < c_segments; ++i)
	{
		float a1 = TWO_PI * float(i) / c_segments;
		float a2 = TWO_PI * float(i + 1) / c_segments;

		float x1 = cx + std::cos(a1) * m_brushRadius;
		float z1 = cz + std::sin(a1) * m_brushRadius;

		float x2 = cx + std::cos(a2) * m_brushRadius;
		float z2 = cz + std::sin(a2) * m_brushRadius;

		float y1 = m_compositor->getBilinearHeight(x1, z1);
		float y2 = m_compositor->getBilinearHeight(x2, z2);

		primitiveRenderer->drawLine(
			Vector4(x1, y1 + c_offset, z1, 1.0f),
			Vector4(x2, y2 + c_offset, z2, 1.0f),
			2.0f,
			brushColor
		);
	}

	float y = m_compositor->getBilinearHeight(cx, cz);
	primitiveRenderer->drawSolidPoint(Vector4(cx, y + c_offset, cz, 1.0f), 4.0f, brushColor);
}

TerrainEntityEditor::TerrainEntityEditor(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	hf::HeightfieldCompositor* compositor
)
:	scene::DefaultEntityEditor(context, entityAdapter)
,	m_compositor(compositor)
,	m_strokeLast(Vector4::origo())
,	m_brushType(0)
,	m_brushRadius(4.0f)
{
}

void TerrainEntityEditor::updateHeightfield(bool patches, bool normals, bool heights, const hf::Region& dirtyRegion)
{
	TerrainEntity* terrainEntity = checked_type_cast< TerrainEntity* >(getEntityAdapter()->getEntity());

	resource::Proxy< hf::Heightfield > heightfield = terrainEntity->getHeightfield();
	if (!heightfield.validate())
		return;

	hf::height_t* resourceHeights = const_cast< hf::height_t* >(heightfield->getHeights());
	m_compositor->copyHeights(resourceHeights);

	if (patches)
		terrainEntity->updatePatches();

	if (normals || heights)
		terrainEntity->updateTextures(normals, heights);
}

	}
}
