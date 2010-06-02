#include <cmath>
#include "Scene/Editor/Modifiers/RotateModifier.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Render/PrimitiveRenderer.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.RotateModifier", RotateModifier, IModifier)

void RotateModifier::draw(
	SceneEditorContext* context,
	const Matrix44& viewTransform,
	const Transform& worldTransform,
	render::PrimitiveRenderer* primitiveRenderer,
	int button
)
{
	const float guideRadius = 2.0f;

	uint32_t axisEnable = context->getAxisEnable();

	const Vector4& translation = worldTransform.translation();
	const Quaternion& rotation = worldTransform.rotation();

	float head, pitch, bank;
	rotation.toEulerAngles(head, pitch, bank);

	Matrix44 mh = rotateY(head);
	Matrix44 mp = rotateX(pitch);
	Matrix44 mb = rotateZ(bank);

	//primitiveRenderer->pushDepthEnable(false);

	// Head
	primitiveRenderer->pushWorld(translate(translation) * mh);

	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * guideRadius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * guideRadius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * guideRadius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * guideRadius;

		primitiveRenderer->drawLine(
			Vector4(c1, 0.0f, s1, 0.0f),
			Vector4(c2, 0.0f, s2, 0.0f),
			axisEnable & SceneEditorContext::AeX ? 2.0f : 0.0f,
			Color(255, 0, 0)
		);
	}

	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, 0.0f, guideRadius, 1.0f),
		axisEnable & SceneEditorContext::AeX ? 2.0f : 0.0f,
		Color(255, 0, 0)
	);

	primitiveRenderer->popWorld();

	// Pitch
	primitiveRenderer->pushWorld(translate(translation) * mh * mp);

	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * guideRadius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * guideRadius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * guideRadius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * guideRadius;

		primitiveRenderer->drawLine(
			Vector4(0.0f, c1, s1, 0.0f),
			Vector4(0.0f, c2, s2, 0.0f),
			axisEnable & SceneEditorContext::AeY ? 2.0f : 0.0f,
			Color(0, 255, 0)
		);
	}

	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, 0.0f, guideRadius, 1.0f),
		axisEnable & SceneEditorContext::AeY ? 2.0f : 0.0f,
		Color(0, 255, 0)
	);

	primitiveRenderer->popWorld();

	// Bank
	primitiveRenderer->pushWorld(translate(translation) * mh * mp * mb);

	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * guideRadius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * guideRadius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * guideRadius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * guideRadius;

		primitiveRenderer->drawLine(
			Vector4(c1, s1, 0.0f, 0.0f),
			Vector4(c2, s2, 0.0f, 0.0f),
			axisEnable & SceneEditorContext::AeZ ? 2.0f : 0.0f,
			Color(0, 0, 255)
		);
	}

	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, guideRadius, 0.0f, 1.0f),
		axisEnable & SceneEditorContext::AeZ ? 2.0f : 0.0f,
		Color(0, 0, 255)
	);

	primitiveRenderer->popWorld();

	//primitiveRenderer->popDepthEnable();
}

void RotateModifier::adjust(
	SceneEditorContext* context,
	const Matrix44& viewTransform,
	const Vector4& screenDelta,
	const Vector4& viewDelta,
	const Vector4& worldDelta,
	int button,
	Transform& outTransform
)
{
	const float c_constantDeltaScale = 0.02f;
	uint32_t axisEnable = context->getAxisEnable();

	float head, pitch, bank;
	outTransform.rotation().toEulerAngles(head, pitch, bank);

	if ((axisEnable & SceneEditorContext::AeX) != 0 && button == 0)
		head += screenDelta.x() * c_constantDeltaScale;
	if ((axisEnable & SceneEditorContext::AeY) != 0 && button == 0)
		pitch += screenDelta.y() * c_constantDeltaScale;
	if ((axisEnable & SceneEditorContext::AeZ) != 0 && button != 0)
		bank += screenDelta.x() * c_constantDeltaScale;

	//Matrix44 mt = translate(outTransform.translation());
	//Matrix44 mh = rotateY(head);
	//Matrix44 mp = rotateX(pitch);
	//Matrix44 mb = rotateZ(bank);

	//outTransform = Transform(mt * mh * mp * mb);

	outTransform = Transform(
		outTransform.translation(),
		Quaternion(head, pitch, bank)
	);
}

	}
}
