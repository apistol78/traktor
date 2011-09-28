#include "Core/Math/Const.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Terrain/Heightfield.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "Terrain/Editor/TerrainEntityEditor.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEntityEditor", TerrainEntityEditor, scene::DefaultEntityEditor)

TerrainEntityEditor::TerrainEntityEditor(scene::SceneEditorContext* context)
:	scene::DefaultEntityEditor(context)
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
	return false;
}

void TerrainEntityEditor::drawGuide(
	scene::SceneEditorContext* context,
	render::PrimitiveRenderer* primitiveRenderer,
	scene::EntityAdapter* entityAdapter
) const
{
	Ref< TerrainEntity > terrainEntity = checked_type_cast< TerrainEntity* >(entityAdapter->getEntity());

	resource::Proxy< Heightfield > heightfield = terrainEntity->getHeightfield();
	if (!heightfield.validate())
		return;

	const int32_t c_segments = 30;
	const float c_radius = 4.0;
	const float c_offset = 0.5f;

	for (int i = 0; i < c_segments; ++i)
	{
		float a1 = TWO_PI * float(i) / c_segments;
		float a2 = TWO_PI * float(i + 1) / c_segments;

		float x1 = std::cos(a1) * c_radius;
		float z1 = std::sin(a1) * c_radius;

		float x2 = std::cos(a2) * c_radius;
		float z2 = std::sin(a2) * c_radius;

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

	}
}
