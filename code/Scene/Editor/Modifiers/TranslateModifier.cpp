#include "Scene/Editor/Modifiers/TranslateModifier.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.TranslateModifier", TranslateModifier, IModifier)

void TranslateModifier::draw(
	SceneEditorContext* context,
	const Matrix44& viewTransform,
	const Transform& worldTransform,
	render::PrimitiveRenderer* primitiveRenderer,
	int button
)
{
	const Scalar c_guideScale(0.1f);
	const Scalar c_guideMinLength(1.0f);
	const float c_infinite = 1e4f;

	Scalar cameraDistance = (worldTransform.toMatrix44() * viewTransform).translation().length();
	Scalar guideLength = max(cameraDistance * c_guideScale, c_guideMinLength);
	Scalar arrowLength = guideLength * Scalar(1.0f / 8.0f);

	primitiveRenderer->pushView(viewTransform);
	primitiveRenderer->pushWorld(translate(worldTransform.translation()));

	uint32_t axisEnable = context->getAxisEnable();

	primitiveRenderer->pushDepthEnable(true);

	primitiveRenderer->drawLine(
		Vector4(-c_infinite, 0.0f, 0.0f, 1.0f),
		Vector4(c_infinite, 0.0f, 0.0f, 1.0f),
		1.0f,
		Color4ub(255, 0, 0, 100)
	);
	primitiveRenderer->drawLine(
		Vector4(0.0f, -c_infinite, 0.0f, 1.0f),
		Vector4(0.0f, c_infinite, 0.0f, 1.0f),
		1.0f,
		Color4ub(0, 255, 0, 100)
	);
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, -c_infinite, 1.0f),
		Vector4(0.0f, 0.0f, c_infinite, 1.0f),
		1.0f,
		Color4ub(0, 0, 255, 100)
	);

	primitiveRenderer->popDepthEnable();
	primitiveRenderer->pushDepthEnable(false);

	if (axisEnable & SceneEditorContext::AeX)
	{
		primitiveRenderer->drawLine(
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(guideLength, 0.0f, 0.0f, 1.0f),
			3.0f,
			Color4ub(255, 0, 0, 255)
		);
		primitiveRenderer->drawArrowHead(
			Vector4(guideLength, 0.0f, 0.0f, 1.0f),
			Vector4(guideLength + arrowLength, 0.0f, 0.0f, 1.0f),
			0.5f,
			Color4ub(255, 0, 0, 255)
		);
	}

	if (axisEnable & SceneEditorContext::AeY)
	{
		primitiveRenderer->drawLine(
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, guideLength, 0.0f, 1.0f),
			3.0f,
			Color4ub(0, 255, 0, 255)
		);
		primitiveRenderer->drawArrowHead(
			Vector4(0.0f, guideLength, 0.0f, 1.0f),
			Vector4(0.0f, guideLength + arrowLength, 0.0f, 1.0f),
			0.5f,
			Color4ub(0, 255, 0, 255)
		);
	}

	if (axisEnable & SceneEditorContext::AeZ)
	{
		primitiveRenderer->drawLine(
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, guideLength, 1.0f),
			3.0f,
			Color4ub(0, 0, 255, 255)
		);
		primitiveRenderer->drawArrowHead(
			Vector4(0.0f, 0.0f, guideLength, 1.0f),
			Vector4(0.0f, 0.0f, guideLength + arrowLength, 1.0f),
			0.5f,
			Color4ub(0, 0, 255, 255)
		);
	}

	primitiveRenderer->popDepthEnable();
	primitiveRenderer->popWorld();
	primitiveRenderer->popView();
}

void TranslateModifier::adjust(
	SceneEditorContext* context,
	const Matrix44& viewTransform,
	const Vector4& screenDelta,
	const Vector4& viewDelta,
	const Vector4& worldDelta,
	int button,
	Transform& outTransform
)
{
	uint32_t axisEnable = context->getAxisEnable();

	Vector4 worldDeltaAdjust = worldDelta;
	if ((axisEnable & SceneEditorContext::AeX) == 0)
		worldDeltaAdjust *= Vector4(0.0f, 1.0f, 1.0f);
	if ((axisEnable & SceneEditorContext::AeY) == 0)
		worldDeltaAdjust *= Vector4(1.0f, 0.0f, 1.0f);
	if ((axisEnable & SceneEditorContext::AeZ) == 0)
		worldDeltaAdjust *= Vector4(1.0f, 1.0f, 0.0f);

	outTransform = Transform(worldDeltaAdjust) * outTransform;
}

	}
}
