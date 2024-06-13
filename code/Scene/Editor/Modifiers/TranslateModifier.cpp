/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <numeric>
#include "Core/Log/Log.h"
#include "Core/Math/Line2.h"
#include "Core/Math/Winding2.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/Modifiers/TranslateModifier.h"
#include "Ui/Application.h"
#include "Ui/Command.h"

namespace traktor::scene
{
	namespace
	{

const float c_guideThickness(0.02f);
const Scalar c_guideScale(0.15f);
const Scalar c_guideMinLength(1.0f);
const float c_infinite = 1e4f;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.TranslateModifier", TranslateModifier, IModifier)

TranslateModifier::TranslateModifier(SceneEditorContext* context)
:	m_context(context)
,	m_axisEnable(0)
,	m_axisHot(0)
{
}

bool TranslateModifier::activate()
{
	return true;
}

void TranslateModifier::deactivate()
{
}

void TranslateModifier::selectionChanged()
{
	m_entityAdapters = m_context->getEntities(SceneEditorContext::GfDefault | SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfNoExternalChild);

	m_baseTranslations.clear();
	for (auto entityAdapter : m_entityAdapters)
	{
		const Transform T = entityAdapter->getTransform();
		m_baseTranslations.push_back(T.translation());
	}

	m_center = Vector4::origo();
	if (!m_baseTranslations.empty())
	{
		m_center = Vector4::zero();
		for (const auto& baseTranslation : m_baseTranslations)
			m_center += baseTranslation;

		m_center /= Scalar(float(m_baseTranslations.size()));
		m_center = snap(m_center.xyz1(), 1 | 2 | 4, false);
	}
}

void TranslateModifier::buttonDown()
{
}

IModifier::CursorMovedResult TranslateModifier::cursorMoved(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection
)
{
	if (m_entityAdapters.empty())
		return { false, false };

	const Vector4 eye = transformChain.getView().inverse().translation();
	const Scalar distance = (m_center - eye).xyz0().length();

	const float axisLength = (distance / 4.0f) * m_context->getGuideSize();
	const float arrowLength = axisLength / 7.0f;
	const float squareLength = axisLength / 2.5f;

	TransformChain tc = transformChain;
	tc.pushWorld(translate(m_center));

	const Vector4 viewDirection = m_center - eye;
	const float sx = viewDirection.x() < 0.0f ? 1.0f : -1.0f;
	const float sy = viewDirection.y() < 0.0f ? 1.0f : -1.0f;
	const float sz = viewDirection.z() < 0.0f ? 1.0f : -1.0f;

	Vector2 center, axis0[3], axis1[3], square[6];
	tc.objectToScreen(Vector4(0.0f, 0.0f, 0.0f, 1.0f), center);
	tc.objectToScreen(Vector4(-axisLength, 0.0f, 0.0f, 1.0f), axis0[0]);
	tc.objectToScreen(Vector4(0.0f, -axisLength, 0.0f, 1.0f), axis0[1]);
	tc.objectToScreen(Vector4(0.0f, 0.0f, -axisLength, 1.0f), axis0[2]);
	tc.objectToScreen(Vector4(axisLength + arrowLength, 0.0f, 0.0f, 1.0f), axis1[0]);
	tc.objectToScreen(Vector4(0.0f, axisLength + arrowLength, 0.0f, 1.0f), axis1[1]);
	tc.objectToScreen(Vector4(0.0f, 0.0f, axisLength + arrowLength, 1.0f), axis1[2]);
	tc.objectToScreen(Vector4(sx * squareLength, 0.0f, 0.0f, 1.0f), square[0]);
	tc.objectToScreen(Vector4(0.0f, sy * squareLength, 0.0f, 1.0f), square[1]);
	tc.objectToScreen(Vector4(0.0f, 0.0f, sz * squareLength, 1.0f), square[2]);
	tc.objectToScreen(Vector4(sx * squareLength, sy * squareLength, 0.0f, 1.0f), square[3]);
	tc.objectToScreen(Vector4(sx * squareLength, 0.0f, sz * squareLength, 1.0f), square[4]);
	tc.objectToScreen(Vector4(0.0f, sy * squareLength, sz * squareLength, 1.0f), square[5]);

	tc.popWorld();

	const uint32_t lastAxisHot = m_axisHot;
	m_axisHot = 0;

	// Check drag circles.
	{
		const float d = (center - cursorPosition).length();
		if (d < 0.015f)
			m_axisHot |= 1 | 2 | 4;
	}

	// Check combo squares.
	{
		Winding2 w(4);

		// XY
		if (m_axisHot == 0)
		{
			w[0] = center;
			w[1] = square[0];
			w[2] = square[3];
			w[3] = square[1];
			if (w.inside(cursorPosition))
				m_axisHot |= 1 | 2;
		}

		// XZ
		if (m_axisHot == 0)
		{
			w[0] = center;
			w[1] = square[0];
			w[2] = square[4];
			w[3] = square[2];
			if (w.inside(cursorPosition))
				m_axisHot |= 1 | 4;
		}

		// YZ
		if (m_axisHot == 0)
		{
			w[0] = center;
			w[1] = square[1];
			w[2] = square[5];
			w[3] = square[2];
			if (w.inside(cursorPosition))
				m_axisHot |= 2 | 4;
		}
	}

	// Check each line.
	if (m_axisHot == 0)
	{
		const float guideThickness = c_guideThickness;
		if (Line2(axis0[0], axis1[0]).classify(cursorPosition, guideThickness))
			m_axisHot |= 1;
		if (Line2(axis0[1], axis1[1]).classify(cursorPosition, guideThickness))
			m_axisHot |= 2;
		if (Line2(axis0[2], axis1[2]).classify(cursorPosition, guideThickness))
			m_axisHot |= 4;
	}

	return { m_axisHot != 0, m_axisHot != lastAxisHot };
}

bool TranslateModifier::handleCommand(const ui::Command& command)
{
	if (command == L"Scene.Editor.Snap")
	{
		for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
		{
			Transform T = m_entityAdapters[i]->getTransform0();
			m_entityAdapters[i]->setTransform(Transform(
				snap(m_baseTranslations[i], 1 | 2 | 4, false),
				T.rotation()
			));
		}
		selectionChanged();
		return true;
	}
	else
	{
		float offset = 0.0f;

		if (command == L"Scene.Editor.SnapToNext")
			offset = m_context->getSnapSpacing();
		else if (command == L"Scene.Editor.SnapToPrevious")
			offset = -m_context->getSnapSpacing();
		else
			return false;

		const Vector4 delta(
			(m_axisHot & 1) ? offset : 0.0f,
			(m_axisHot & 2) ? offset : 0.0f,
			(m_axisHot & 4) ? offset : 0.0f,
			0.0f
		);

		for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
		{
			const Transform T = m_entityAdapters[i]->getTransform0();
			m_entityAdapters[i]->setTransform(Transform(
				snap(m_baseTranslations[i] + delta, m_axisHot, false),
				T.rotation()
			));
		}

		selectionChanged();
		return true;
	}
}

bool TranslateModifier::begin(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	int32_t mouseButton
)
{
	m_axisEnable = m_axisHot;
	m_center0 = m_center;
	return true;
}

void TranslateModifier::apply(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	const Vector4& screenDelta,
	const Vector4& viewDelta,
	bool snapOverrideEnable
)
{
	const Vector4 cp = transformChain.worldToClip(m_center);

	if (m_axisEnable != (1 | 2 | 4))
	{
		Vector4 worldDelta = transformChain.getView().inverse() * viewDelta * cp.w();
		if (!(m_axisEnable & 1))
			worldDelta *= Vector4(0.0f, 1.0f, 1.0f);
		if (!(m_axisEnable & 2))
			worldDelta *= Vector4(1.0f, 0.0f, 1.0f);
		if (!(m_axisEnable & 4))
			worldDelta *= Vector4(1.0f, 1.0f, 0.0f);
		m_center += worldDelta;

		Vector4 baseDelta = snap(m_center, 1 | 2 | 4, snapOverrideEnable) - m_center0;
		if (!(m_axisEnable & 1))
			baseDelta *= Vector4(0.0f, 1.0f, 1.0f);
		if (!(m_axisEnable & 2))
			baseDelta *= Vector4(1.0f, 0.0f, 1.0f);
		if (!(m_axisEnable & 4))
			baseDelta *= Vector4(1.0f, 1.0f, 0.0f);

		for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
		{
			const Transform T = m_entityAdapters[i]->getTransform();
			m_entityAdapters[i]->setTransform(Transform(
				snap(m_baseTranslations[i] + baseDelta, m_axisEnable, snapOverrideEnable),
				T.rotation()
			));
		}
	}
	else
	{
		Vector4 worldDelta = transformChain.getView().inverse() * viewDelta * cp.w();
		m_center += worldDelta;

		Vector4 baseDelta = m_center - m_center0;

		for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
		{
			const Transform T = m_entityAdapters[i]->getTransform();
			m_entityAdapters[i]->setTransform(Transform(
				snap(m_baseTranslations[i] + baseDelta, 1 | 2 | 4, snapOverrideEnable),
				T.rotation()
			));
		}
	}

	m_axisHot = m_axisEnable;
}

void TranslateModifier::end(const TransformChain& transformChain)
{
	selectionChanged();
	m_axisEnable = 0;
}

void TranslateModifier::draw(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (m_entityAdapters.empty())
		return;

	const Vector4 eye = primitiveRenderer->getView().inverse().translation();
	const Scalar distance = (m_center - eye).xyz0().length();

	const float axisLength = (distance / 4.0f) * m_context->getGuideSize();
	const float arrowLength = axisLength / 7.0f;
	const float squareLength = axisLength / 2.5f;

	primitiveRenderer->pushWorld(translate(m_center));

	// Infinite "trace" lines.
	primitiveRenderer->pushDepthState(true, false, false);
	if (m_axisHot & 1)
		primitiveRenderer->drawLine(
			Vector4(-c_infinite, 0.0f, 0.0f, 1.0f),
			Vector4(c_infinite, 0.0f, 0.0f, 1.0f),
			1.0f,
			Color4ub(255, 0, 0, 100)
		);
	if (m_axisHot & 2)
		primitiveRenderer->drawLine(
			Vector4(0.0f, -c_infinite, 0.0f, 1.0f),
			Vector4(0.0f, c_infinite, 0.0f, 1.0f),
			1.0f,
			Color4ub(0, 255, 0, 100)
		);
	if (m_axisHot & 4)
		primitiveRenderer->drawLine(
			Vector4(0.0f, 0.0f, -c_infinite, 1.0f),
			Vector4(0.0f, 0.0f, c_infinite, 1.0f),
			1.0f,
			Color4ub(0, 0, 255, 100)
		);
	primitiveRenderer->popDepthState();

	primitiveRenderer->pushDepthState(false, false, false);

	// Drag circles.
	primitiveRenderer->drawWireCircle(
		Vector4(axisLength + arrowLength * 3.0f, 0.0f, 0.0f, 1.0f),
		Vector4(1.0f, 0.0f, 0.0f, 0.0f),
		0.2f,
		(float)((m_axisHot & 1) ? 3 : 1),
		Color4ub(255, 0, 0, 255)
	);
	primitiveRenderer->drawWireCircle(
		Vector4(0.0f, axisLength + arrowLength * 3.0f, 0.0f, 1.0f),
		Vector4(0.0f, 1.0f, 0.0f, 0.0f),
		0.2f,
		(float)((m_axisHot & 2) ? 3 : 1),
		Color4ub(0, 255, 0, 255)
	);
	primitiveRenderer->drawWireCircle(
		Vector4(0.0f, 0.0f, axisLength + arrowLength * 3.0f, 1.0f),
		Vector4(0.0f, 0.0f, 1.0f, 0.0f),
		0.2f,
		(float)((m_axisHot & 4) ? 3 : 1),
		Color4ub(0, 0, 255, 255)
	);

	const Vector4 viewDirection = m_center - primitiveRenderer->getView().inverse().translation();
	const float sx = viewDirection.x() < 0.0f ? 1.0f : -1.0f;
	const float sy = viewDirection.y() < 0.0f ? 1.0f : -1.0f;
	const float sz = viewDirection.z() < 0.0f ? 1.0f : -1.0f;

	// Guide fill squares.
	// XY
	if (m_axisEnable == 0 || m_axisEnable == (1 | 2))
	{
		primitiveRenderer->drawSolidQuad(
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(sx * squareLength, 0.0f, 0.0f, 1.0f),
			Vector4(sx * squareLength, sy * squareLength, 0.0f, 1.0f),
			Vector4(0.0f, sy * squareLength, 0.0f, 1.0f),
			Color4ub(255, 255, 0, m_axisHot == (1 | 2) ? 90 : 70)
		);
	}
	// XZ
	if (m_axisEnable == 0 || m_axisEnable == (1 | 4))
	{
		primitiveRenderer->drawSolidQuad(
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(sx * squareLength, 0.0f, 0.0f, 1.0f),
			Vector4(sx * squareLength, 0.0f, sz * squareLength, 1.0f),
			Vector4(0.0f, 0.0f, sz * squareLength, 1.0f),
			Color4ub(255, 255, 0, m_axisHot == (1 | 4) ? 90 : 70)
		);
	}
	// YZ
	if (m_axisEnable == 0 || m_axisEnable == (2 | 4))
	{
		primitiveRenderer->drawSolidQuad(
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, sy * squareLength, 0.0f, 1.0f),
			Vector4(0.0f, sy * squareLength, sz * squareLength, 1.0f),
			Vector4(0.0f, 0.0f, sz * squareLength, 1.0f),
			Color4ub(255, 255, 0, m_axisHot == (2 | 4) ? 90 : 70)
		);
	}

	// Guide square lines.
	// XY
	if (m_axisEnable == 0 || m_axisEnable == (1 | 2))
	{
		const float awxy = (float)(((m_axisHot & (1 | 2)) == (1 | 2)) ? 3 : 1);
		primitiveRenderer->drawLine(Vector4(sx * squareLength, 0.0f, 0.0f, 1.0f), Vector4(sx * squareLength, sy * squareLength, 0.0f, 1.0f), awxy, Color4ub(255, 0, 0, 255));
		primitiveRenderer->drawLine(Vector4(0.0f, sy * squareLength, 0.0f, 1.0f), Vector4(sx * squareLength, sy * squareLength, 0.0f, 1.0f), awxy, Color4ub(0, 255, 0, 255));
	}

	// XZ
	if (m_axisEnable == 0 || m_axisEnable == (1 | 4))
	{
		const float awxz = (float)(((m_axisHot & (1 | 4)) == (1 | 4)) ? 3 : 1);
		primitiveRenderer->drawLine(Vector4(sx * squareLength, 0.0f, 0.0f, 1.0f), Vector4(sx * squareLength, 0.0f, sz * squareLength, 1.0f), awxz, Color4ub(255, 0, 0, 255));
		primitiveRenderer->drawLine(Vector4(0.0f, 0.0f, sz * squareLength, 1.0f), Vector4(sx * squareLength, 0.0f, sz * squareLength, 1.0f), awxz, Color4ub(0, 0, 255, 255));
	}

	// YZ
	if (m_axisEnable == 0 || m_axisEnable == (2 | 4))
	{
		const float awyz = (float)(((m_axisHot & (2 | 4)) == (2 | 4)) ? 3 : 1);
		primitiveRenderer->drawLine(Vector4(0.0f, sy * squareLength, 0.0f, 1.0f), Vector4(0.0f, sy * squareLength, sz * squareLength, 1.0f), awyz, Color4ub(0, 255, 0, 255));
		primitiveRenderer->drawLine(Vector4(0.0f, 0.0f, sz * squareLength, 1.0f), Vector4(0.0f, sy * squareLength, sz * squareLength, 1.0f), awyz, Color4ub(0, 0, 255, 255));
	}

	// Guide axis lines.
	primitiveRenderer->drawLine(
		Vector4(-axisLength, 0.0f, 0.0f, 1.0f),
		Vector4(axisLength, 0.0f, 0.0f, 1.0f),
		(float)((m_axisHot & 1) ? 3 : 1),
		Color4ub(255, 0, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(0.0f, -axisLength, 0.0f, 1.0f),
		Vector4(0.0f, axisLength, 0.0f, 1.0f),
		(float)((m_axisHot & 2) ? 3 : 1),
		Color4ub(0, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, -axisLength, 1.0f),
		Vector4(0.0f, 0.0f, axisLength, 1.0f),
		(float)((m_axisHot & 4) ? 3 : 1),
		Color4ub(0, 0, 255, 255)
	);

	// Guide arrows.
	primitiveRenderer->drawArrowHead(
		Vector4(axisLength, 0.0f, 0.0f, 1.0f),
		Vector4(axisLength + arrowLength, 0.0f, 0.0f, 1.0f),
		0.5f,
		Color4ub(255, 0, 0, 255)
	);
	primitiveRenderer->drawArrowHead(
		Vector4(0.0f, axisLength, 0.0f, 1.0f),
		Vector4(0.0f, axisLength + arrowLength, 0.0f, 1.0f),
		0.5f,
		Color4ub(0, 255, 0, 255)
	);
	primitiveRenderer->drawArrowHead(
		Vector4(0.0f, 0.0f, axisLength, 1.0f),
		Vector4(0.0f, 0.0f, axisLength + arrowLength, 1.0f),
		0.5f,
		Color4ub(0, 0, 255, 255)
	);

	// Center point.
	primitiveRenderer->drawSolidPoint(
		Vector4::origo(),
		10.0f,
		(m_axisHot != (1 | 2 | 4)) ? Color4ub(255, 255, 255, 180) : Color4ub(255, 255, 255, 255)
	);

	primitiveRenderer->popDepthState();

	primitiveRenderer->popWorld();
}

Vector4 TranslateModifier::snap(const Vector4& position, uint32_t axisEnable, bool snapOverrideEnable) const
{
	if (m_context->getSnapMode() == SceneEditorContext::SmGrid && !snapOverrideEnable)
	{
		const float spacing = m_context->getSnapSpacing();
		if (spacing > 0.0f)
		{
			return Vector4(
				(axisEnable & 1) ? floor(position[0] / spacing + 0.5f) * spacing : position[0],
				(axisEnable & 2) ? floor(position[1] / spacing + 0.5f) * spacing : position[1],
				(axisEnable & 4) ? floor(position[2] / spacing + 0.5f) * spacing : position[2],
				1.0f
			);
		}
	}
	return position;
}

}
