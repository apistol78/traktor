#include "Core/Math/Const.h"
#include "Core/Math/Random.h"
#include "Terrain/OceanEntity.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/Editor/OceanEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.OceanEntityEditor", OceanEntityEditor, scene::DefaultEntityEditor)

OceanEntityEditor::OceanEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}


bool OceanEntityEditor::handleCommand(const ui::Command& command)
{
	Ref< OceanEntityData > oceanEntityData = checked_type_cast< OceanEntityData* >(getEntityAdapter()->getEntityData());

	if (command == L"Ocean.RandomizeWaves")
	{
		static Random s_random;

		float globalAngle = s_random.nextFloat() * TWO_PI;

		for (int i = 0; i < OceanEntityData::MaxWaves; ++i)
		{
			static float maxRandomAngle = deg2rad(40.0f);
			static float maxAmplitude = 0.5f;

			float localAngle = (s_random.nextFloat() - 0.5f) * maxRandomAngle + globalAngle;

			float frequency = TWO_PI * (i + 1);

			float fu = sin(localAngle) * frequency;
			float fv = cos(localAngle) * frequency;

			fu = floor(fu / TWO_PI) * TWO_PI;
			fv = floor(fv / TWO_PI) * TWO_PI;

			float f = sqrt(fu * fu + fv * fv);

			OceanEntityData::Wave wave;
			wave.direction.x = cos(localAngle) * f;
			wave.direction.y = sin(localAngle) * f;
			wave.amplitude = maxAmplitude * pow(1.0f - float(i) / OceanEntityData::MaxWaves, 4.0f);
			wave.phase = s_random.nextFloat() * TWO_PI;

			oceanEntityData->setWave(i, wave);
		}

		getContext()->buildEntities();
	}
	else
		return false;

	return true;
}

void OceanEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
}

	}
}
