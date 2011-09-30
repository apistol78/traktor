#include <limits>
#include "Core/Math/Const.h"
#include "Editor/IEditor.h"
#include "Heightfield/Heightfield.h"
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
	return m_compositor->queryRay(worldRayOrigin, worldRayDirection, outDistance, &m_lastQueryIntersection);
}

void TerrainEntityEditor::beginModifier()
{
	m_strokeBegin = m_lastQueryIntersection;
	m_strokeEnd = m_lastQueryIntersection;
}

void TerrainEntityEditor::applyModifier(const ApplyParams& params)
{
	Vector4 position;
	Scalar distance;

	if (m_compositor->queryRay(params.worldRayOrigin, params.worldRayDirection, distance, &position))
		m_strokeEnd = position;
}

void TerrainEntityEditor::endModifier()
{
}

bool TerrainEntityEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Heightfield.Raise")
		return applyHeightfieldModifier(1.0f);
	else if (command == L"Heightfield.Lower")
		return applyHeightfieldModifier(-1.0f);
	else
		return false;

	return true;
}

void TerrainEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	const int32_t c_segments = 30;
	const float c_radius = 4.0f;
	const float c_offset = 0.5f;

	float cx = m_strokeBegin.x();
	float cz = m_strokeBegin.z();

	for (int32_t i = 0; i < c_segments; ++i)
	{
		float a1 = TWO_PI * float(i) / c_segments;
		float a2 = TWO_PI * float(i + 1) / c_segments;

		float x1 = cx + std::cos(a1) * c_radius;
		float z1 = cz + std::sin(a1) * c_radius;

		float x2 = cx + std::cos(a2) * c_radius;
		float z2 = cz + std::sin(a2) * c_radius;

		float y1 = m_compositor->getBilinearHeight(x1, z1);
		float y2 = m_compositor->getBilinearHeight(x2, z2);

		primitiveRenderer->drawLine(
			Vector4(x1, y1 + c_offset, z1, 1.0f),
			Vector4(x2, y2 + c_offset, z2, 1.0f),
			2.0f,
			Color4ub(255, 255, 0, 255)
		);
	}

	int32_t steps = int32_t(std::ceil(16.0f * (m_strokeEnd - m_strokeBegin).length()));;
	for (int32_t i = 0; i < steps; ++i)
	{
		Vector4 p1 = lerp(m_strokeBegin, m_strokeEnd, Scalar(float(i) / steps));
		Vector4 p2 = lerp(m_strokeBegin, m_strokeEnd, Scalar(float(i + 1) / steps));

		float y1 = m_compositor->getBilinearHeight(p1.x(), p1.z());
		float y2 = m_compositor->getBilinearHeight(p2.x(), p2.z());

		primitiveRenderer->drawLine(
			Vector4(p1.x(), y1 + c_offset, p1.z(), 1.0f),
			Vector4(p2.x(), y2 + c_offset, p2.z(), 1.0f),
			2.0f,
			Color4ub(255, 255, 0, 255)
		);
	}
}

TerrainEntityEditor::TerrainEntityEditor(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	hf::HeightfieldCompositor* compositor
)
:	scene::DefaultEntityEditor(context, entityAdapter)
,	m_compositor(compositor)
,	m_lastQueryIntersection(Vector4::origo())
,	m_strokeBegin(Vector4::origo())
,	m_strokeEnd(Vector4::origo())
{
}

bool TerrainEntityEditor::applyHeightfieldModifier(float pressure)
{
	TerrainEntity* terrainEntity = checked_type_cast< TerrainEntity* >(getEntityAdapter()->getEntity());

	resource::Proxy< hf::Heightfield > heightfield = terrainEntity->getHeightfield();
	if (!heightfield.validate())
		return false;

	hf::RoundBrush brush(4.0f, pressure);
	m_compositor->strokeBrush(m_strokeBegin, m_strokeEnd, &brush);

	hf::height_t* resourceHeights = const_cast< hf::height_t* >(heightfield->getHeights());
	m_compositor->copyHeights(resourceHeights);

	terrainEntity->createRenderPatches();

	return true;
}

	}
}
