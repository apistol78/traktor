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

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEntityEditor", TerrainEntityEditor, scene::DefaultEntityEditor)

TerrainEntityEditor::TerrainEntityEditor()
:	m_followGround(false)
,	m_followHeight(20.0f)
{
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
)
{
	if (selected)
	{
		m_followGround = false;
		m_followHeight = 20.0f;			// 1/10th of a meter.
	}
}

void TerrainEntityEditor::applyModifier(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	const Matrix44& viewTransform,
	const Vector4& screenDelta,
	const Vector4& viewDelta,
	const Vector4& worldDelta,
	int mouseButton
)
{
}

bool TerrainEntityEditor::handleCommand(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	const ui::Command& command
)
{
	if (command == L"Terrain.ToggleFollowGround")
	{
		m_followGround = !m_followGround;
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

	//if (m_followGround)
	//{
	//	Ref< scene::Camera > camera = context->getCamera();
	//	T_ASSERT (camera);

	//	Vector4 cameraPosition = camera->getTargetPosition();

	//	cameraPosition =
	//		cameraPosition * Vector4(1.0f, 0.0f, 1.0f, 1.0f) +
	//		Scalar(terrainEntity->getHeightfield()->getWorldHeight(cameraPosition.x(), cameraPosition.z()) + m_followHeight) * Vector4(0.0f, 1.0f, 0.0f, 0.0f);

	//	camera->setTargetPosition(cameraPosition);
	//}
}

	}
}
