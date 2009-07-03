#include "Terrain/Editor/TerrainEntityEditor.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "Terrain/Heightfield.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/Camera.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

struct TerrainData : public Object
{
	bool followGround;
	float followHeight;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEntityEditor", TerrainEntityEditor, scene::IEntityEditor)

TypeSet TerrainEntityEditor::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< TerrainEntityData >());
	return typeSet;
}

bool TerrainEntityEditor::isPickable(
	scene::EntityAdapter* entityAdapter
) const
{
	return false;
}

void TerrainEntityEditor::entitySelected(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	bool selected
) const
{
	if (selected)
	{
		Ref< TerrainData > terrainData = gc_new< TerrainData >();
		terrainData->followGround = false;
		terrainData->followHeight = 20.0f;			// 1/10th of a meter.
		entityAdapter->setUserObject(terrainData);
	}
	else
		entityAdapter->setUserObject(0);
}

void TerrainEntityEditor::applyModifier(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	const Matrix44& viewTransform,
	const Vector2& mouseDelta,
	int mouseButton
) const
{
}

bool TerrainEntityEditor::handleCommand(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	const ui::Command& command
) const
{
	Ref< TerrainData > terrainData = entityAdapter->getUserObject< TerrainData >();
	T_ASSERT (terrainData);

	if (command == L"Terrain.ToggleFollowGround")
	{
		terrainData->followGround = !terrainData->followGround;
	}
	else if (command == L"Terrain.FlushSurfaceCache")
	{
		Ref< TerrainEntity > terrainEntity = checked_type_cast< TerrainEntity* >(entityAdapter->getEntity());
		terrainEntity->getSurfaceCache()->flush();
	}
	else
		return false;

	return true;
}

void TerrainEntityEditor::drawGuide(
	scene::SceneEditorContext* context,
	render::PrimitiveRenderer* primitiveRenderer,
	scene::EntityAdapter* entityAdapter
) const
{
	Ref< TerrainEntity > terrainEntity = checked_type_cast< TerrainEntity* >(entityAdapter->getEntity());
	Ref< TerrainData > terrainData = entityAdapter->getUserObject< TerrainData >();

	if (terrainData && terrainData->followGround)
	{
		Ref< scene::Camera > camera = context->getCamera();
		T_ASSERT (camera);

		Vector4 cameraPosition = camera->getTargetPosition();

		cameraPosition =
			cameraPosition * Vector4(1.0f, 0.0f, 1.0f, 1.0f) +
			Scalar(terrainEntity->getHeightfield()->getWorldHeight(cameraPosition.x(), cameraPosition.z()) + terrainData->followHeight) * Vector4(0.0f, 1.0f, 0.0f, 0.0f);

		camera->setTargetPosition(cameraPosition);
	}
}

bool TerrainEntityEditor::getStatusText(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	std::wstring& outStatusText
) const
{
	return false;
}

	}
}
