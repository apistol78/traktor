#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Shape/Spline/SplineComponent.h"
#include "Shape/Editor/Spline/SplineComponentEditor.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineComponentEditor", SplineComponentEditor, scene::DefaultComponentEditor)

SplineComponentEditor::SplineComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData)
:	scene::DefaultComponentEditor(context, entityAdapter, componentData)
{
}

void SplineComponentEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	auto splineComponent = m_entityAdapter->getComponent< SplineComponent >();
	if (!splineComponent)
		return;

	primitiveRenderer->pushDepthState(false, false, false);

	const auto& path = splineComponent->getPath();
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

	primitiveRenderer->popDepthState();
}

	}
}
