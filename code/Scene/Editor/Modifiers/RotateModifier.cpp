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
	Vector4 ax = m.axisX();
	outHead = std::atan2(ax.z(), ax.x());

	Vector4 az = m.axisZ();
	Vector4 yz = rotateY(-outHead) * az;
	outPitch = std::atan2(yz.y(), yz.z());

	Vector4 ay = m.axisY();
	Vector4 xy = rotateX(outPitch) * (rotateY(-outHead) * ay);
	outBank = -std::atan2(xy.x(), xy.y());
}
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.RotateModifier", RotateModifier, IModifier)

void RotateModifier::draw(
	SceneEditorContext* context,
	const Matrix44& viewTransform,
	const Matrix44& worldTransform,
	render::PrimitiveRenderer* primitiveRenderer,
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

	Matrix44 mh = rotateY(head);
	Matrix44 mp = rotateZ(pitch);
	Matrix44 mb = rotateZ(bank);

	primitiveRenderer->pushDepthEnable(false);

	// Head
	primitiveRenderer->pushWorld(mh * translate(translation));

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
	primitiveRenderer->pushWorld(mp * rotateY(PI / 2.0f) * mh * translate(translation));

	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * guideRadius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * guideRadius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * guideRadius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * guideRadius;

		primitiveRenderer->drawLine(
			Vector4(c1, s1, 0.0f, 0.0f),
			Vector4(c2, s2, 0.0f, 0.0f),
			axisEnable & SceneEditorContext::AeY ? 2.0f : 0.0f,
			Color(0, 255, 0)
		);
	}

	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(guideRadius, 0.0f, 0.0f, 1.0f),
		axisEnable & SceneEditorContext::AeY ? 2.0f : 0.0f,
		Color(0, 255, 0)
	);

	primitiveRenderer->popWorld();

	// Bank
	primitiveRenderer->pushWorld(mb * mh * translate(translation));

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
		Vector4(guideRadius, 0.0f, 0.0f, 1.0f),
		axisEnable & SceneEditorContext::AeZ ? 2.0f : 0.0f,
		Color(0, 0, 255)
	);

	primitiveRenderer->popWorld();

	primitiveRenderer->popDepthEnable();
}

void RotateModifier::adjust(
	SceneEditorContext* context,
	const Matrix44& viewTransform,
	const Vector4& screenDelta,
	const Vector4& viewDelta,
	const Vector4& worldDelta,
	int button,
	Matrix44& outTransform
)
{
	const float c_constantDeltaScale = 0.02f;
	uint32_t axisEnable = context->getAxisEnable();

	Matrix44 R = outTransform;
	//R(3) = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

	Matrix44 mdh = (button == 0) ? rotateY(screenDelta.x() * c_constantDeltaScale) : Matrix44::identity();
	Matrix44 mdp = (button == 0) ? rotateX(screenDelta.y() * c_constantDeltaScale) : Matrix44::identity();
	Matrix44 mdb = (button != 0) ? rotateZ(screenDelta.x() * c_constantDeltaScale) : Matrix44::identity();

	Matrix44 T = R * mdh * (R.inverse() * mdb * R) * (R.inverse() * mdp * R);

	outTransform = Matrix44(
		T.axisX().normalized(),
		T.axisY().normalized(),
		T.axisZ().normalized(),
		outTransform.translation()
	);
}

	}
}
