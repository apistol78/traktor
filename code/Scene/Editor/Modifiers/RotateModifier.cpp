#include <cmath>
#include "Scene/Editor/Modifiers/RotateModifier.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Render/PrimitiveRenderer.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{
		
void getEulerAngles(
	const Matrix44& m,
	float& outHead,
	float& outPitch,
	float& outBank
)
{
	outHead = std::atan2(m.e[0][2], m.e[2][2]);
	outPitch = -std::asin(-m.e[1][2]);
	outBank = -std::atan2(m.e[1][0], m.e[1][1]);
}
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.RotateModifier", RotateModifier, Modifier)

void RotateModifier::draw(
	SceneEditorContext* context,
	const Matrix44& worldTransform,
	render::PrimitiveRenderer* primitiveRenderer,
	bool active,
	int button
)
{
	const float c_guideScale = 0.1f;

	uint32_t axisEnable = context->getAxisEnable();

	float cameraDistance = (worldTransform * primitiveRenderer->getView()).translation().length();
	float guideRadius = cameraDistance * c_guideScale;

	Vector4 translation = worldTransform.translation();

	float head, pitch, bank;
	getEulerAngles(worldTransform, head, pitch, bank);

	Matrix44 mh = rotateY(head) * translate(translation);
	Matrix44 mp = rotateX(pitch) * rotateY(head) * translate(translation);
	Matrix44 mb = rotateZ(bank) * rotateX(pitch) * rotateY(head) * translate(translation);

	// Head
	primitiveRenderer->pushWorld(mh);

	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * guideRadius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * guideRadius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * guideRadius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * guideRadius;

		primitiveRenderer->drawLine(
			Vector4(c1, 0.0f, s1, 0.0f),
			Vector4(c2, 0.0f, s2, 0.0f),
			axisEnable & SceneEditorContext::AeX ? 4.0f : 0.0f,
			Color(255, 0, 0)
		);
	}

	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, 0.0f, guideRadius, 1.0f),
		axisEnable & SceneEditorContext::AeX ? 4.0f : 0.0f,
		Color(255, 0, 0)
	);

	primitiveRenderer->popWorld();

	// Pitch
	primitiveRenderer->pushWorld(mp);

	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * guideRadius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * guideRadius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * guideRadius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * guideRadius;

		primitiveRenderer->drawLine(
			Vector4(0.0f, s1, c1, 0.0f),
			Vector4(0.0f, s2, c2, 0.0f),
			axisEnable & SceneEditorContext::AeY ? 4.0f : 0.0f,
			Color(0, 255, 0)
		);
	}

	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, guideRadius, 0.0f, 1.0f),
		axisEnable & SceneEditorContext::AeY ? 4.0f : 0.0f,
		Color(0, 255, 0)
	);

	primitiveRenderer->popWorld();

	// Bank
	primitiveRenderer->pushWorld(mb);

	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * guideRadius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * guideRadius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * guideRadius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * guideRadius;

		primitiveRenderer->drawLine(
			Vector4(c1, s1, 0.0f, 0.0f),
			Vector4(c2, s2, 0.0f, 0.0f),
			axisEnable & SceneEditorContext::AeZ ? 4.0f : 0.0f,
			Color(0, 0, 255)
		);
	}

	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(guideRadius, 0.0f, 0.0f, 1.0f),
		axisEnable & SceneEditorContext::AeZ ? 4.0f : 0.0f,
		Color(0, 0, 255)
	);

	primitiveRenderer->popWorld();
}

void RotateModifier::adjust(
	SceneEditorContext* context,
	const Matrix44& viewTransform,
	const Vector2& screenDelta,
	int button,
	Matrix44& outTransform
)
{
	const float c_constantDeltaScale = 0.02f;
	uint32_t axisEnable = context->getAxisEnable();

	float head, pitch, bank;
	getEulerAngles(outTransform, head, pitch, bank);

	Matrix44 mp = rotateY(head);
	Matrix44 mb = rotateX(pitch) * rotateY(head);

	Quaternion delta = Quaternion::identity();

	if (button == 0)
	{
		if (axisEnable & SceneEditorContext::AeX)
			delta *= Quaternion(Vector4(0.0f, screenDelta.x * c_constantDeltaScale, 0.0f, 0.0f));
		if (axisEnable & SceneEditorContext::AeY)
			delta *= Quaternion(mp * Vector4(screenDelta.y * c_constantDeltaScale, 0.0f, 0.0f, 0.0f));
	}
	else
	{
		if (axisEnable & SceneEditorContext::AeZ)
			delta *= Quaternion(mb * Vector4(0.0f, 0.0f, screenDelta.x * c_constantDeltaScale, 0.0f));
	}

	outTransform = delta.normalized().toMatrix44() * outTransform;
}

	}
}
