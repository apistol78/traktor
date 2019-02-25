#include "Core/Log/Log.h"
#include "Core/Math/TransformPath.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Shape/Spline/ControlPointEntityData.h"
#include "Shape/Spline/SplineEntityData.h"
#include "Shape/Editor/Spline/SplineEntityEditor.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineEntityEditor", SplineEntityEditor, scene::DefaultEntityEditor)

SplineEntityEditor::SplineEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

bool SplineEntityEditor::isGroup() const
{
	return true;
}

bool SplineEntityEditor::addChildEntity(scene::EntityAdapter* childEntityAdapter) const
{
	world::EntityData* entityData = getEntityAdapter()->getEntityData();
	world::EntityData* childEntityData = childEntityAdapter->getEntityData();

	if (SplineEntityData* splineEntityData = dynamic_type_cast< SplineEntityData* >(entityData))
	{
		ControlPointEntityData* controlPointEntityData = dynamic_type_cast< ControlPointEntityData* >(childEntityData);
		if (!controlPointEntityData)
		{
			log::error << L"Only entities of ControlPointEntityData allowed as children to SplineEntityData." << Endl;
			return false;
		}

		splineEntityData->addEntityData(controlPointEntityData);
		return true;
	}

	return false;
}

void SplineEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	const SplineEntityData* splineEntityData = checked_type_cast< const SplineEntityData* >(getEntityAdapter()->getEntityData());

	primitiveRenderer->pushDepthState(false, false, false);

	const auto& controlPointEntities = splineEntityData->getControlPointEntities();
	if (controlPointEntities.size() >= 2)
	{
		TransformPath path;
		for (uint32_t i = 0; i < controlPointEntities.size(); ++i)
		{
			const auto& T = controlPointEntities[i]->getTransform();

			TransformPath::Key k;
			k.T = (float)i / (controlPointEntities.size() - 1);
			k.position = T.translation();
			k.orientation = T.rotation().toEulerAngles();
			path.insert(k);
		}

		const auto& keys = path.getKeys();

		float st = keys.front().T;
		float et = keys.back().T;

		uint32_t nsteps = keys.size() * 10;
		for (uint32_t i = 0; i < nsteps; ++i)
		{
			float t1 = st + (float)(i * (et - st)) / nsteps;
			float t2 = st + (float)((i + 1) * (et - st)) / nsteps;
			primitiveRenderer->drawLine(
				path.evaluate(t1).position,
				path.evaluate(t2).position,
				Color4ub(170, 170, 255, 255)
			);
		}
	}	

	primitiveRenderer->popDepthState();

	scene::DefaultEntityEditor::drawGuide(primitiveRenderer);
}

	}
}
