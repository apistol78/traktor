/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Core/Math/Const.h"
#include "Core/Math/Line2.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/Modifiers/RotateModifier.h"
#include "Ui/Application.h"
#include "Ui/Command.h"

namespace traktor::scene
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

bool RotateModifier::activate()
{
	return true;
}

void RotateModifier::deactivate()
{
}

void RotateModifier::selectionChanged()
{
	m_entityAdapters = m_context->getEntities(SceneEditorContext::GfDefault | SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfNoExternalChild);

	m_baseTransforms.clear();

	m_center = Vector4::zero();
	for (auto entityAdapter : m_entityAdapters)
	{
		const Transform T = entityAdapter->getTransform();
		m_baseTransforms.push_back(T);
		m_center += T.translation();
	}
	if (!m_entityAdapters.empty())
		m_center /= Scalar(float(m_entityAdapters.size()));
	m_center = m_center.xyz1();

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

void RotateModifier::buttonDown()
{
}

IModifier::CursorMovedResult RotateModifier::cursorMoved(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection
)
{
	if (m_entityAdapters.empty())
		return { false, false };

	const Matrix44 mh = rotateY(m_baseHead + m_deltaHead);
	const Matrix44 mp = rotateX(m_basePitch + m_deltaPitch);
	const Matrix44 mb = rotateZ(m_baseBank + m_deltaBank);

	const Vector4 eye = transformChain.getView().inverse().translation();
	const Scalar distance = (m_center - eye).xyz0().length();
	const Scalar radius = (distance / 6.0_simd) * Scalar(m_context->getGuideSize());

	const uint32_t lastAxisEnable = m_axisEnable;
	m_axisEnable = 0;

	TransformChain tc = transformChain;
	tc.pushWorld(translate(m_center) * mh);
	for (int i = 0; i < 64; ++i)
	{
		const float s1 = std::sin(2.0f * PI * i / 64.0f) * radius;
		const float c1 = std::cos(2.0f * PI * i / 64.0f) * radius;
		const float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * radius;
		const float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * radius;

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
		const float s1 = std::sin(2.0f * PI * i / 64.0f) * radius;
		const float c1 = std::cos(2.0f * PI * i / 64.0f) * radius;
		const float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * radius;
		const float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * radius;

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
		const float s1 = std::sin(2.0f * PI * i / 64.0f) * radius;
		const float c1 = std::cos(2.0f * PI * i / 64.0f) * radius;
		const float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * radius;
		const float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * radius;

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

	return { m_axisEnable != 0, m_axisEnable != lastAxisEnable };
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
		const Quaternion Q = Quaternion::fromEulerAngles(m_baseHead + m_deltaHead, m_basePitch + m_deltaPitch, m_baseBank + m_deltaBank);

		const Transform T = m_entityAdapters.front()->getTransform();
		const Transform Tn(T.translation(), Q);

		m_entityAdapters.front()->setTransform(Tn);
	}
	else
	{
		const Quaternion Q = Quaternion::fromEulerAngles(m_deltaHead, m_deltaPitch, m_deltaBank);

		const Transform Tc(m_center.xyz1());
		const Transform Tci(-m_center.xyz1());

		for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
		{
			const Transform T0 = Tci * m_baseTransforms[i];
			const Transform T1 = Transform(Q) * T0;
			const Transform T2 = Tc * T1;
			m_entityAdapters[i]->setTransform(T2);
		}
	}

	return true;
}

bool RotateModifier::begin(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	int32_t mouseButton
)
{
	m_deltaHead = 0.0f;
	m_deltaPitch = 0.0f;
	m_deltaBank = 0.0f;
	return true;
}

void RotateModifier::apply(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	const Vector4& screenDelta,
	const Vector4& viewDelta,
	bool snapOverrideEnable
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
		float head = m_baseHead + m_deltaHead;
		float pitch = m_basePitch + m_deltaPitch;
		float bank = m_baseBank + m_deltaBank;

		if (snapOverrideEnable)
		{
			const float snapAngle = deg2rad(45.0f);
			if ((m_axisEnable & 1) != 0)
				head = std::floor(head / snapAngle + 0.5f) * snapAngle;
			if ((m_axisEnable & 2) != 0)
				pitch = std::floor(pitch / snapAngle + 0.5f) * snapAngle;
			if ((m_axisEnable & 4) != 0)
				bank = std::floor(bank / snapAngle + 0.5f) * snapAngle;
		}

		const Quaternion Q = Quaternion::fromEulerAngles(head, pitch, bank);

		const Transform T = m_baseTransforms.front();
		const Transform Tn(T.translation(), Q);

		m_entityAdapters.front()->setTransform(Tn);
	}
	else
	{
		const Quaternion Q = Quaternion::fromEulerAngles(m_deltaHead, m_deltaPitch, m_deltaBank);

		const Transform Tc(m_center.xyz1());
		const Transform Tci(-m_center.xyz1());

		for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
		{
			const Transform T0 = Tci * m_baseTransforms[i];
			const Transform T1 = Transform(Q) * T0;
			const Transform T2 = Tc * T1;
			m_entityAdapters[i]->setTransform(T2);
		}
	}
}

void RotateModifier::end(const TransformChain& transformChain)
{
	m_baseTransforms.clear();
	for (auto entityAdapter : m_entityAdapters)
	{
		const Transform T = entityAdapter->getTransform();
		m_baseTransforms.push_back(T);
	}

	if (m_baseTransforms.size() == 1)
		m_baseTransforms.front().rotation().toEulerAngles(m_baseHead, m_basePitch, m_baseBank);
	else
	{
		m_baseHead = 0.0f;
		m_basePitch = 0.0f;
		m_baseBank = 0.0f;
	}

	m_deltaHead = 0.0f;
	m_deltaPitch = 0.0f;
	m_deltaBank = 0.0f;
}

void RotateModifier::draw(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (m_entityAdapters.empty())
		return;

	const Vector4 eye = primitiveRenderer->getView().inverse().translation();
	const Scalar distance = (m_center - eye).xyz0().length();
	const Scalar radius = (distance / 6.0_simd) * Scalar(m_context->getGuideSize());

	const Matrix44 mh = rotateY(m_baseHead + m_deltaHead);
	const Matrix44 mp = rotateX(m_basePitch + m_deltaPitch);
	const Matrix44 mb = rotateZ(m_baseBank + m_deltaBank);

	primitiveRenderer->pushDepthState(false, false, false);

	// Head
	primitiveRenderer->pushWorld(translate(m_center) * mh);
	for (int i = 0; i < 64; ++i)
	{
		const float s1 = std::sin(2.0f * PI * i / 64.0f) * radius;
		const float c1 = std::cos(2.0f * PI * i / 64.0f) * radius;
		const float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * radius;
		const float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * radius;

		primitiveRenderer->drawLine(
			Vector4(c1, 0.0f, s1, 1.0f),
			Vector4(c2, 0.0f, s2, 1.0f),
			(float)((m_axisEnable & 1) ? 3 : 1),
			Color4ub(255, 0, 0)
		);
	}
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, 0.0f, radius, 1.0f),
		(float)((m_axisEnable & 1) ? 3 : 1),
		Color4ub(255, 0, 0)
	);
	primitiveRenderer->popWorld();

	// Pitch
	primitiveRenderer->pushWorld(translate(m_center) * mh * mp);
	for (int i = 0; i < 64; ++i)
	{
		const float s1 = std::sin(2.0f * PI * i / 64.0f) * radius;
		const float c1 = std::cos(2.0f * PI * i / 64.0f) * radius;
		const float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * radius;
		const float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * radius;

		primitiveRenderer->drawLine(
			Vector4(0.0f, c1, s1, 1.0f),
			Vector4(0.0f, c2, s2, 1.0f),
			(float)((m_axisEnable & 2) ? 3 : 1),
			Color4ub(0, 255, 0)
		);
	}
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, 0.0f, radius, 1.0f),
		(float)((m_axisEnable & 2) ? 3 : 1),
		Color4ub(0, 255, 0)
	);
	primitiveRenderer->popWorld();

	// Bank
	primitiveRenderer->pushWorld(translate(m_center) * mh * mp * mb);
	for (int i = 0; i < 64; ++i)
	{
		const float s1 = std::sin(2.0f * PI * i / 64.0f) * radius;
		const float c1 = std::cos(2.0f * PI * i / 64.0f) * radius;
		const float s2 = std::sin(2.0f * PI * (i + 1) / 64.0f) * radius;
		const float c2 = std::cos(2.0f * PI * (i + 1) / 64.0f) * radius;

		primitiveRenderer->drawLine(
			Vector4(c1, s1, 0.0f, 1.0f),
			Vector4(c2, s2, 0.0f, 1.0f),
			(float)((m_axisEnable & 4) ? 3 : 1),
			Color4ub(0, 0, 255)
		);
	}
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, radius, 0.0f, 1.0f),
		(float)((m_axisEnable & 4) ? 3 : 1),
		Color4ub(0, 0, 255)
	);
	primitiveRenderer->popWorld();

	primitiveRenderer->popDepthState();
}

}
