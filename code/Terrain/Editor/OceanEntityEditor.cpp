#include "Terrain/Editor/OceanEntityEditor.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/OceanEntity.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Ui/Command.h"
#include "Core/Math/Random.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.OceanEntityEditor", OceanEntityEditor, scene::DefaultEntityEditor)

bool OceanEntityEditor::isPickable(
	scene::EntityAdapter* entityAdapter
) const
{
	return false;
}

void OceanEntityEditor::applyModifier(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	const Matrix44& viewTransform,
	const Vector2& mouseDelta,
	int mouseButton
)
{
}

bool OceanEntityEditor::handleCommand(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	const ui::Command& command
)
{
	Ref< OceanEntityData > oceanEntityData = checked_type_cast< OceanEntityData* >(entityAdapter->getEntityData());

	if (command == L"Ocean.RandomizeWaves")
	{
		static Random s_random;

		float globalAngle = s_random.nextFloat() * PI * 2.0f;

		for (int i = 0; i < OceanEntityData::MaxWaves; ++i)
		{
			static float maxRandomAngle = deg2rad(20.0f);
			static float repeatDuration = 0.03f;
			static float maxAmplitude = 0.5f;

			float localAngle = (s_random.nextFloat() - 0.5f) * maxRandomAngle + globalAngle;

			float baseFrequency = 2.0f * PI / repeatDuration;
			float frequency = baseFrequency * (i + 1);

			OceanEntityData::Wave wave;
			wave.direction.x = cos(localAngle) * frequency;
			wave.direction.y = sin(localAngle) * frequency;
			wave.amplitude = maxAmplitude * pow(1.0f - float(i) / OceanEntityData::MaxWaves, 4.0f);
			wave.phase = s_random.nextFloat() * PI * 2.0f;

			oceanEntityData->setWave(i, wave);
		}

		context->buildEntities();
	}
	else
		return false;

	return true;
}

void OceanEntityEditor::drawGuide(
	scene::SceneEditorContext* context,
	render::PrimitiveRenderer* primitiveRenderer,
	scene::EntityAdapter* entityAdapter
) const
{
}

	}
}
