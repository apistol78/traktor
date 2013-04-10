#include "Heightfield/Heightfield.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/Editor/TerrainEditModifier.h"
#include "Terrain/Editor/TerrainEntityEditor.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEntityEditor", TerrainEntityEditor, scene::DefaultEntityEditor)

Ref< TerrainEntityEditor > TerrainEntityEditor::create(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
{
	return new TerrainEntityEditor(context, entityAdapter);
}

bool TerrainEntityEditor::isPickable() const
{
	return false;
}

bool TerrainEntityEditor::queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const
{
	return false;
}

void TerrainEntityEditor::entitySelected(bool selected)
{
	if (selected)
		getContext()->setModifier(new TerrainEditModifier(getContext()));
	else
		getContext()->setModifier(0);
}

bool TerrainEntityEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void TerrainEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	TerrainEntity* terrainEntity = checked_type_cast< TerrainEntity* >(getEntityAdapter()->getEntity());

	const resource::Proxy< hf::Heightfield >& heightfield = terrainEntity->getTerrain()->getHeightfield();

	const Vector4& worldExtent = heightfield->getWorldExtent();

	int32_t x0 = int32_t(-worldExtent.x() / 2.0f);
	int32_t x1 = int32_t( worldExtent.x() / 2.0f);
	int32_t z0 = int32_t(-worldExtent.z() / 2.0f);
	int32_t z1 = int32_t( worldExtent.z() / 2.0f);

	const float c_epsilon = 0.1f;

	for (int32_t x = x0; x <= x1; x += 10)
	{
		for (int32_t z = z0; z < z1; z += 1)
		{
			float y0 = heightfield->getWorldHeight(float(x), float(z));
			float y1 = heightfield->getWorldHeight(float(x), float(z + 1));

			primitiveRenderer->drawLine(
				Vector4(x, y0 + c_epsilon, z, 1.0f),
				Vector4(x, y1 + c_epsilon, z + 1, 1.0f),
				1.0f,
				Color4ub(255, 255, 255, 80)
			);
		}
	}

	for (int32_t z = z0; z <= z1; z += 10)
	{
		for (int32_t x = x0; x < x1; x += 1)
		{
			float y0 = heightfield->getWorldHeight(float(x), float(z));
			float y1 = heightfield->getWorldHeight(float(x + 1), float(z));

			primitiveRenderer->drawLine(
				Vector4(x, y0 + c_epsilon, z, 1.0f),
				Vector4(x + 1, y1 + c_epsilon, z, 1.0f),
				1.0f,
				Color4ub(255, 255, 255, 80)
			);
		}
	}
}

TerrainEntityEditor::TerrainEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

	}
}
