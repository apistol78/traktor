#include <cmath>
#include "Core/Math/Const.h"
#include "Core/Math/Line2.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/Modifiers/RotateModifier.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const float c_guideThickness(0.015f);
const Scalar c_guideScale(0.15f);
const Scalar c_guideMinRadius(1.0f);

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.RotateModifier", RotateModifier, IModifier)

RotateModifier::RotateModifier(SceneEditorContext* context)
:	m_context(context)
,	m_deltaHead(0.0f)
,	m_deltaPitch(0.0f)
,	m_deltaBank(0.0f)
,	m_baseHead(0.0f)
,	m_basePitch(0.0f)
,	m_baseBank(0.0f)
,	m_axisEnable(0)
{
}

void RotateModifier::selectionChanged()
{
	m_entityAdapters.clear();
	m_context->getEntities(m_entityAdapters, SceneEditorContext::GfDefault | SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfNoExternalChild);

	m_baseTransforms.clear();
	m_center = Vector4::zero();

	for (RefArray< EntityAdapter >::const_iterator i = m_entityAdapters.begin(); i != m_entityAdapters.end(); ++i)
	{
		Transform T = (*i)->getTransform();
		m_baseTransforms.push_back(T);
		m_center += T.translation();
	}

	if (!m_entityAdapters.empty())
		m_center /= Scalar(float(m_entityAdapters.size()));

	m_center = m_center.xyz1();

	m_deltaHead = 0.0f;
	m_deltaPitch = 0.0f;
	m_deltaBank = 0.0f;

	if (m_baseTransforms.size() == 1)
		m_baseTransforms.front().rotation().toEulerAngles(m_baseHead, m_basePitch, m_baseBank);
	else
	{
		m_baseHead = 0.0f;
		m_basePitch = 0.0f;
		m_baseBank = 0.0f;
	}

	m_axisEnable = 0;
}

bool RotateModifier::cursorMoved(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection
)
{
	if (m_entityAdapters.empty())
		return false;

	Matrix44 mh = rotateY(m_baseHead + m_deltaHead);
	Matrix44 mp = rotateX(m_basePitch + m_deltaPitch);
	Matrix44 mb = rotateZ(m_baseBank + m_deltaBank);

	Scalar radius = Scalar(m_context->getGuideSize());

	m_axisEnable = 0;

	TransformChain tc = transformChain;
	tc.pushWorld(translate(m_center) * mh);
	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * radius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * radius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * radius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * radius;

		Vector2 s, e;
		tc.objectToScreen(Vector4(c1, 0.0f, s1, 1.0f), s);
		tc.objectToScreen(Vector4(c2, 0.0f, s2, 1.0f), e);

		if (Line2(s, e).classify(cursorPosition, c_guideThickness))
		{
			m_axisEnable |= 1;
			break;
		}
	}
	tc.popWorld();

	tc.pushWorld(translate(m_center) * mh * mp);
	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * radius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * radius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * radius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * radius;

		Vector2 s, e;
		tc.objectToScreen(Vector4(0.0f, c1, s1, 1.0f), s);
		tc.objectToScreen(Vector4(0.0f, c2, s2, 1.0f), e);

		if (Line2(s, e).classify(cursorPosition, c_guideThickness))
		{
			m_axisEnable |= 2;
			break;
		}
	}
	tc.popWorld();

	tc.pushWorld(translate(m_center) * mh * mp * mb);
	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * radius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * radius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * radius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * radius;

		Vector2 s, e;
		tc.objectToScreen(Vector4(c1, s1, 0.0f, 1.0f), s);
		tc.objectToScreen(Vector4(c2, s2, 0.0f, 1.0f), e);

		if (Line2(s, e).classify(cursorPosition, c_guideThickness))
		{
			m_axisEnable |= 4;
			break;
		}
	}
	tc.popWorld();

	return m_axisEnable != 0;
}

bool RotateModifier::handleCommand(const ui::Command& command)
{
	float rotate = 0.0f;

	if (command == L"Scene.Editor.RotatePlus30")
		rotate = deg2rad(30.0f);
	else if (command == L"Scene.Editor.RotatePlus45")
		rotate = deg2rad(45.0f);
	else if (command == L"Scene.Editor.RotatePlus90")
		rotate = deg2rad(90.0f);
	else if (command == L"Scene.Editor.RotateMinus30")
		rotate = deg2rad(-30.0f);
	else if (command == L"Scene.Editor.RotateMinus45")
		rotate = deg2rad(-45.0f);
	else if (command == L"Scene.Editor.RotateMinus90")
		rotate = deg2rad(-90.0f);
	else
		return false;

	if ((m_axisEnable & 1) != 0)
		m_deltaHead += rotate;
	if ((m_axisEnable & 2) != 0)
		m_deltaPitch += rotate;
	if ((m_axisEnable & 4) != 0)
		m_deltaBank += rotate;

	if (m_entityAdapters.size() == 1)
	{
		Quaternion Q = Quaternion::fromEulerAngles(m_baseHead + m_deltaHead, m_basePitch + m_deltaPitch, m_baseBank + m_deltaBank);

		Transform T = m_entityAdapters.front()->getTransform();
		Transform Tn(T.translation(), Q);

		m_entityAdapters.front()->setTransform(Tn);
	}
	else
	{
		Quaternion Q = Quaternion::fromEulerAngles(m_deltaHead, m_deltaPitch, m_deltaBank);

		Transform Tc(m_center.xyz1());
		Transform Tci(-m_center.xyz1());

		for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
		{
			Transform T0 = Tci * m_baseTransforms[i];
			Transform T1 = Transform(Q) * T0;
			Transform T2 = Tc * T1;
			m_entityAdapters[i]->setTransform(T2);
		}
	}

	return true;
}

bool RotateModifier::begin(
	const TransformChain& transformChain,
	int32_t mouseButton
)
{
	return true;
}

void RotateModifier::apply(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	const Vector4& screenDelta,
	const Vector4& viewDelta
)
{
	const float c_constantDeltaScale = 0.02f;

	float dHead = 0.0f, dPitch = 0.0f, dBank = 0.0f;
	if ((m_axisEnable & 1) != 0)
		dHead = screenDelta.x() * c_constantDeltaScale;
	if ((m_axisEnable & 2) != 0)
		dPitch = screenDelta.y() * c_constantDeltaScale;
	if ((m_axisEnable & 4) != 0)
		dBank = screenDelta.y() * c_constantDeltaScale;

	m_deltaHead += dHead;
	m_deltaPitch += dPitch;
	m_deltaBank += dBank;

	if (m_entityAdapters.size() == 1)
	{
		Quaternion Q = Quaternion::fromEulerAngles(m_baseHead + m_deltaHead, m_basePitch + m_deltaPitch, m_baseBank + m_deltaBank);

		Transform T = m_entityAdapters.front()->getTransform();
		Transform Tn(T.translation(), Q);

		m_entityAdapters.front()->setTransform(Tn);
	}
	else
	{
		Quaternion Q = Quaternion::fromEulerAngles(m_deltaHead, m_deltaPitch, m_deltaBank);

		Transform Tc(m_center.xyz1());
		Transform Tci(-m_center.xyz1());

		for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
		{
			Transform T0 = Tci * m_baseTransforms[i];
			Transform T1 = Transform(Q) * T0;
			Transform T2 = Tc * T1;
			m_entityAdapters[i]->setTransform(T2);
		}
	}
}

void RotateModifier::end(const TransformChain& transformChain)
{
}

void RotateModifier::draw(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (m_entityAdapters.empty())
		return;

	Scalar radius = Scalar(m_context->getGuideSize());

	Matrix44 mh = rotateY(m_baseHead + m_deltaHead);
	Matrix44 mp = rotateX(m_basePitch + m_deltaPitch);
	Matrix44 mb = rotateZ(m_baseBank + m_deltaBank);

	primitiveRenderer->pushDepthState(false, false);

	// Head
	primitiveRenderer->pushWorld(translate(m_center) * mh);
	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * radius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * radius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * radius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * radius;

		primitiveRenderer->drawLine(
			Vector4(c1, 0.0f, s1, 1.0f),
			Vector4(c2, 0.0f, s2, 1.0f),
			(m_axisEnable & 1) ? 3.0f : 1.0f,
			Color4ub(255, 0, 0)
		);
	}
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, 0.0f, radius, 1.0f),
		(m_axisEnable & 1) ? 3.0f : 1.0f,
		Color4ub(255, 0, 0)
	);
	primitiveRenderer->popWorld();

	// Pitch
	primitiveRenderer->pushWorld(translate(m_center) * mh * mp);
	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * radius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * radius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * radius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * radius;

		primitiveRenderer->drawLine(
			Vector4(0.0f, c1, s1, 1.0f),
			Vector4(0.0f, c2, s2, 1.0f),
			(m_axisEnable & 2) ? 3.0f : 1.0f,
			Color4ub(0, 255, 0)
		);
	}
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, 0.0f, radius, 1.0f),
		(m_axisEnable & 2) ? 3.0f : 1.0f,
		Color4ub(0, 255, 0)
	);
	primitiveRenderer->popWorld();

	// Bank
	primitiveRenderer->pushWorld(translate(m_center) * mh * mp * mb);
	for (int i = 0; i < 64; ++i)
	{
		float s1 = std::sin(2.0f * PI * i / 64.0f) * radius;
		float c1 = std::cos(2.0f * PI * i / 64.0f) * radius;
		float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * radius;
		float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * radius;

		primitiveRenderer->drawLine(
			Vector4(c1, s1, 0.0f, 1.0f),
			Vector4(c2, s2, 0.0f, 1.0f),
			(m_axisEnable & 4) ? 3.0f : 1.0f,
			Color4ub(0, 0, 255)
		);
	}
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, radius, 0.0f, 1.0f),
		(m_axisEnable & 4) ? 3.0f : 1.0f,
		Color4ub(0, 0, 255)
	);
	primitiveRenderer->popWorld();

	primitiveRenderer->popDepthState();
}

	}
}
