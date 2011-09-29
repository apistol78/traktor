#include <limits>
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Core/Math/Winding3.h"
#include "Editor/IEditor.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/HeightfieldCompositor.h"
#include "Heightfield/Editor/HeightfieldEditorPlugin.h"
#include "Heightfield/Editor/HeightfieldLayer.h"
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
	Ref< TerrainEntity > terrainEntity = checked_type_cast< TerrainEntity* >(getEntityAdapter()->getEntity());

	resource::Proxy< hf::Heightfield > heightfield = terrainEntity->getHeightfield();
	if (!heightfield.validate())
		return false;

	const hf::HeightfieldResource& resource = heightfield->getResource();
	if (resource.getSize() <= 1)
		return false;

	Vector4 worldExtent = resource.getWorldExtent();
	uint32_t size = resource.getSize();
	uint32_t skip = 4;

	Scalar k;
	Scalar kIn, kOut;

	Aabb3 terrainBoundingBox(-worldExtent * Scalar(0.5f), worldExtent * Scalar(0.5f));
	if (!terrainBoundingBox.intersectRay(worldRayOrigin, worldRayDirection, kIn, kOut))
		return false;

	float dx = worldExtent.x() / (size - 1);
	float dz = worldExtent.z() / (size - 1);

	Winding3 w;
	w.points.resize(3);

	bool foundIntersection = false;

	outDistance = Scalar(std::numeric_limits< float >::max());

	for (uint32_t iz = 0; iz < size; iz += skip)
	{
		float wz = iz * dz - worldExtent.z() * 0.5f;

		for (uint32_t ix = 0; ix < size; ix += skip)
		{
			float wx = ix * dx - worldExtent.x() * 0.5f;

			float heights[] =
			{
				heightfield->getWorldHeight(wx, wz),
				heightfield->getWorldHeight(wx + dx * skip, wz),
				heightfield->getWorldHeight(wx, wz + dz * skip),
				heightfield->getWorldHeight(wx + dx * skip, wz + dz * skip)
			};

			Vector4 wv[] =
			{
				Vector4(wx, heights[0], wz, 1.0f),
				Vector4(wx + dx * skip, heights[1], wz, 1.0f),
				Vector4(wx, heights[2], wz + dz * skip, 1.0f),
				Vector4(wx + dx * skip, heights[3], wz + dz * skip, 1.0f)
			};

			Aabb3 bb;
			bb.contain(wv[0]);
			bb.contain(wv[1]);
			bb.contain(wv[2]);
			bb.contain(wv[3]);

			if (bb.intersectRay(worldRayOrigin, worldRayDirection, kIn, kOut))
			{
				w.points[0] = wv[0];
				w.points[1] = wv[1];
				w.points[2] = wv[2];

				if (w.rayIntersection(worldRayOrigin, worldRayDirection, k))
				{
					if (k < outDistance)
					{
						outDistance = k;
						foundIntersection = true;
					}
				}

				w.points[0] = wv[1];
				w.points[1] = wv[3];
				w.points[2] = wv[2];

				if (w.rayIntersection(worldRayOrigin, worldRayDirection, k))
				{
					if (k < outDistance)
					{
						outDistance = k;
						foundIntersection = true;
					}
				}
			}
		}
	}

	if (foundIntersection)
		m_lastQueryIntersection = worldRayOrigin + worldRayDirection * outDistance;

	return foundIntersection;
}

bool TerrainEntityEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Heightfield.Raise")
		return applyHeightfieldModifier(64.0f);
	else if (command == L"Heightfield.Lower")
		return applyHeightfieldModifier(-64.0f);
	else
		return false;

	return true;
}

void TerrainEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	Ref< TerrainEntity > terrainEntity = checked_type_cast< TerrainEntity* >(getEntityAdapter()->getEntity());

	resource::Proxy< hf::Heightfield > heightfield = terrainEntity->getHeightfield();
	if (!heightfield.validate())
		return;

	const int32_t c_segments = 30;
	const float c_radius = 4.0f;
	const float c_offset = 0.5f;

	float cx = m_lastQueryIntersection.x();
	float cz = m_lastQueryIntersection.z();

	for (int i = 0; i < c_segments; ++i)
	{
		float a1 = TWO_PI * float(i) / c_segments;
		float a2 = TWO_PI * float(i + 1) / c_segments;

		float x1 = cx + std::cos(a1) * c_radius;
		float z1 = cz + std::sin(a1) * c_radius;

		float x2 = cx + std::cos(a2) * c_radius;
		float z2 = cz + std::sin(a2) * c_radius;

		float y1 = heightfield->getWorldHeight(x1, z1);
		float y2 = heightfield->getWorldHeight(x2, z2);

		primitiveRenderer->drawLine(
			Vector4(x1, y1 + c_offset, z1, 1.0f),
			Vector4(x2, y2 + c_offset, z2, 1.0f),
			8.0f,
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
{
}

bool TerrainEntityEditor::applyHeightfieldModifier(float scale)
{
	Ref< TerrainEntity > terrainEntity = checked_type_cast< TerrainEntity* >(getEntityAdapter()->getEntity());

	resource::Proxy< hf::Heightfield > heightfield = terrainEntity->getHeightfield();
	if (!heightfield.validate())
		return false;

	uint32_t size = heightfield->getResource().getSize();

	const float c_radius = 4.0f;

	float cx = m_lastQueryIntersection.x();
	float cz = m_lastQueryIntersection.z();

	float minX, maxX;
	float minZ, maxZ;

	heightfield->worldToGrid(cx - c_radius, cz - c_radius, minX, minZ);
	heightfield->worldToGrid(cx + c_radius, cz + c_radius, maxX, maxZ);

	hf::height_t* heights = m_compositor->getOffsetLayer()->getHeights();
	for (int32_t iz = int32_t(floor(minZ)); iz <= int32_t(ceil(maxZ)); ++iz)
	{
		if (iz < 0 || iz > int32_t(heightfield->getResource().getSize()) - 1)
			continue;

		for (int32_t ix = int32_t(floor(minX)); ix <= int32_t(ceil(maxX)); ++ix)
		{
			if (ix < 0 || ix > int32_t(heightfield->getResource().getSize()) - 1)
				continue;

			int32_t offset = ix + iz * heightfield->getResource().getSize();

			float rx = (ix - (maxX + minX) * 0.5f);
			float rz = (iz - (maxZ + minZ) * 0.5f);
			float distance = clamp(1.0f - sqrt(rx * rx + rz * rz) / (maxX - minX), 0.0f, 1.0f);

			heights[offset] += int32_t(scale * distance);
		}
	}

	Ref< hf::HeightfieldLayer > mergedLayer = m_compositor->mergeLayers();
	if (!mergedLayer)
		return false;

	hf::height_t* resourceHeights = const_cast< hf::height_t* >(heightfield->getHeights());
	const hf::height_t* mergedHeights = mergedLayer->getHeights();

	std::memcpy(resourceHeights, mergedHeights, size * size * sizeof(hf::height_t));

	terrainEntity->createRenderPatches();

	return true;
}

	}
}
