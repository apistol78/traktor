#include "Heightfield/Heightfield.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/TerrainEntityData.h"
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

bool TerrainEntityEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void TerrainEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	//const TerrainEntity* terrainEntity = checked_type_cast< const TerrainEntity*, true >(getEntityAdapter()->getEntity());
	//if (terrainEntity)
	//{
	//	const AlignedVector< TerrainEntity::Patch >& patches = terrainEntity->getPatches();
	//	uint32_t patchCount = terrainEntity->getPatchCount();

	//	const Vector4& worldExtent = terrainEntity->getTerrain()->getHeightfield()->getWorldExtent();
	//	Vector4 patchExtent(worldExtent.x() / float(patchCount), worldExtent.y(), worldExtent.z() / float(patchCount), 0.0f);

	//	Vector4 patchTopLeft = (-worldExtent * Scalar(0.5f)).xyz1();
	//	Vector4 patchDeltaHalf = patchExtent * Vector4(0.5f, 0.0f, 0.5f, 0.0f);
	//	Vector4 patchDeltaX = patchExtent * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	//	Vector4 patchDeltaZ = patchExtent * Vector4(0.0f, 0.0f, 1.0f, 0.0f);

	//	for (uint32_t pz = 0; pz < patchCount; ++pz)
	//	{
	//		Vector4 patchOrigin = patchTopLeft;
	//		for (uint32_t px = 0; px < patchCount; ++px)
	//		{
	//			uint32_t patchId = px + pz * patchCount;
	//			const TerrainEntity::Patch& patch = patches[patchId];

	//			Vector4 patchCenterWorld = (patchOrigin + patchDeltaHalf) * Vector4(1.0f, 0.0f, 1.0f, 0.0f) + Vector4(0.0f, (patch.minHeight + patch.maxHeight) * 0.5f, 0.0f, 1.0f);

	//			Aabb3 patchAabb(
	//				patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f) + Vector4(-patchDeltaHalf.x(), patch.minHeight - FUZZY_EPSILON, -patchDeltaHalf.z(), 0.0f),
	//				patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f) + Vector4( patchDeltaHalf.x(), patch.maxHeight + FUZZY_EPSILON,  patchDeltaHalf.z(), 0.0f)
	//			);

	//			Vector4 extents[8];
	//			patchAabb.getExtents(extents);

	//			primitiveRenderer->drawSolidQuad(
	//				extents[1],
	//				extents[5],
	//				extents[6],
	//				extents[2],
	//				Color4ub(128, 255, 0, 180)
	//			);

	//			primitiveRenderer->drawWireAabb(patchAabb, Color4ub(255, 255, 0, 255));

	//			patchOrigin += patchDeltaX;
	//		}
	//		patchTopLeft += patchDeltaZ;
	//	}
	//}
}

TerrainEntityEditor::TerrainEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

	}
}
