#include "Core/Math/Line2.h"
#include "Core/Math/Winding2.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/Modifiers/TranslateModifier.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const float c_guideThickness(0.015f);
const Scalar c_guideScale(0.15f);
const Scalar c_guideMinLength(1.0f);
const float c_infinite = 1e4f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.TranslateModifier", TranslateModifier, IModifier)

TranslateModifier::TranslateModifier(SceneEditorContext* context)
:	m_context(context)
,	m_axisEnable(0)
{
}

void TranslateModifier::selectionChanged()
{
	m_entityAdapters.clear();
	m_context->getEntities(m_entityAdapters, SceneEditorContext::GfDefault | SceneEditorContext::GfSpatialOnly | SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfNoExternalChild);

	m_baseTranslations.clear();
	for (RefArray< EntityAdapter >::const_iterator i = m_entityAdapters.begin(); i != m_entityAdapters.end(); ++i)
	{
		Transform T = (*i)->getTransform();
		m_baseTranslations.push_back(T.translation());
	}

	m_center = Vector4::zero();
	if (!m_entityAdapters.empty())
	{
		for (RefArray< EntityAdapter >::const_iterator i = m_entityAdapters.begin(); i != m_entityAdapters.end(); ++i)
			m_center += (*i)->getTransform().translation();

		m_center /= Scalar(float(m_entityAdapters.size()));
		m_center = m_center.xyz1();
	}

	m_axisEnable = 0;
}

bool TranslateModifier::cursorMoved(const TransformChain& transformChain, const Vector2& cursorPosition)
{
	if (m_entityAdapters.empty())
		return false;

	float axisLength = m_context->getGuideSize();
	float arrowLength = axisLength / 8.0f;
	float squareLength = axisLength / 3.0f;

	Vector4 snappedCenter = m_center;
	if (m_context->getSnapMode() == SceneEditorContext::SmGrid)
	{
		float spacing = m_context->getSnapSpacing();
		if (spacing > 0.0f)
		{
			snappedCenter.set(
				floor(snappedCenter[0] / spacing + 0.5f) * spacing,
				floor(snappedCenter[1] / spacing + 0.5f) * spacing,
				floor(snappedCenter[2] / spacing + 0.5f) * spacing,
				1.0f
			);
		}
	}

	TransformChain tc = transformChain;
	tc.pushWorld(translate(snappedCenter));

	Vector2 center, axis[3], square[6];
	tc.objectToScreen(Vector4(0.0f, 0.0f, 0.0f, 1.0f), center);
	tc.objectToScreen(Vector4(axisLength + arrowLength, 0.0f, 0.0f, 1.0f), axis[0]);
	tc.objectToScreen(Vector4(0.0f, axisLength + arrowLength, 0.0f, 1.0f), axis[1]);
	tc.objectToScreen(Vector4(0.0f, 0.0f, axisLength + arrowLength, 1.0f), axis[2]);
	tc.objectToScreen(Vector4(squareLength, 0.0f, 0.0f, 1.0f), square[0]);
	tc.objectToScreen(Vector4(0.0f, squareLength, 0.0f, 1.0f), square[1]);
	tc.objectToScreen(Vector4(0.0f, 0.0f, squareLength, 1.0f), square[2]);
	tc.objectToScreen(Vector4(squareLength, squareLength, 0.0f, 1.0f), square[3]);
	tc.objectToScreen(Vector4(squareLength, 0.0f, squareLength, 1.0f), square[4]);
	tc.objectToScreen(Vector4(0.0f, squareLength, squareLength, 1.0f), square[5]);

	tc.popWorld();

	m_axisEnable = 0;

	// First check squares.
	{
		Winding2 w;
		w.p.resize(4);

		// XY
		w.p[0] = center;
		w.p[1] = square[0];
		w.p[2] = square[3];
		w.p[3] = square[1];
		if (w.inside(cursorPosition))
		{
			m_axisEnable |= 1 | 2;
			return true;
		}

		// XZ
		w.p[0] = center;
		w.p[1] = square[0];
		w.p[2] = square[4];
		w.p[3] = square[2];
		if (w.inside(cursorPosition))
		{
			m_axisEnable |= 1 | 4;
			return true;
		}

		// YZ
		w.p[0] = center;
		w.p[1] = square[1];
		w.p[2] = square[5];
		w.p[3] = square[2];
		if (w.inside(cursorPosition))
		{
			m_axisEnable |= 2 | 4;
			return true;
		}
	}

	// If no square hit; check each line.
	if (Line2(center, axis[0]).classify(cursorPosition, c_guideThickness))
		m_axisEnable |= 1;
	if (Line2(center, axis[1]).classify(cursorPosition, c_guideThickness))
		m_axisEnable |= 2;
	if (Line2(center, axis[2]).classify(cursorPosition, c_guideThickness))
		m_axisEnable |= 4;

	return m_axisEnable != 0;
}

bool TranslateModifier::handleCommand(const ui::Command& command)
{
	return false;
}

void TranslateModifier::begin(const TransformChain& transformChain)
{
}

void TranslateModifier::apply(const TransformChain& transformChain, const Vector4& screenDelta, const Vector4& viewDelta)
{
	Vector4 cp = transformChain.worldToClip(m_center);
	Vector4 worldDelta = transformChain.getView().inverse() * viewDelta * cp.w();

	if (!(m_axisEnable & 1))
		worldDelta *= Vector4(0.0f, 1.0f, 1.0f);
	if (!(m_axisEnable & 2))
		worldDelta *= Vector4(1.0f, 0.0f, 1.0f);
	if (!(m_axisEnable & 4))
		worldDelta *= Vector4(1.0f, 1.0f, 0.0f);

	for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
	{
		m_baseTranslations[i] += worldDelta;

		Vector4 translation = m_baseTranslations[i];

		if (m_context->getSnapMode() == SceneEditorContext::SmGrid)
		{
			float spacing = m_context->getSnapSpacing();
			if (spacing > 0.0f)
			{
				translation.set(
					floor(translation[0] / spacing + 0.5f) * spacing,
					floor(translation[1] / spacing + 0.5f) * spacing,
					floor(translation[2] / spacing + 0.5f) * spacing,
					1.0f
				);
			}
		}

		Transform T = m_entityAdapters[i]->getTransform();

		m_entityAdapters[i]->setTransform(Transform(
			translation,
			T.rotation()
		));
	}

	m_center += worldDelta;
}

void TranslateModifier::end(const TransformChain& transformChain)
{
}

void TranslateModifier::draw(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (m_entityAdapters.empty())
		return;

	float axisLength = m_context->getGuideSize();
	float arrowLength = axisLength / 8.0f;
	float squareLength = axisLength / 3.0f;

	Vector4 center = m_center;
	if (m_context->getSnapMode() == SceneEditorContext::SmGrid)
	{
		float spacing = m_context->getSnapSpacing();
		if (spacing > 0.0f)
		{
			center.set(
				floor(center[0] / spacing + 0.5f) * spacing,
				floor(center[1] / spacing + 0.5f) * spacing,
				floor(center[2] / spacing + 0.5f) * spacing,
				1.0f
			);
		}
	}

	primitiveRenderer->pushWorld(translate(center));

	// Infinite "trace" lines.
	primitiveRenderer->pushDepthEnable(true);
	if (m_axisEnable & 1)
		primitiveRenderer->drawLine(
			Vector4(-c_infinite, 0.0f, 0.0f, 1.0f),
			Vector4(c_infinite, 0.0f, 0.0f, 1.0f),
			1.0f,
			Color4ub(255, 0, 0, 100)
		);
	if (m_axisEnable & 2)
		primitiveRenderer->drawLine(
			Vector4(0.0f, -c_infinite, 0.0f, 1.0f),
			Vector4(0.0f, c_infinite, 0.0f, 1.0f),
			1.0f,
			Color4ub(0, 255, 0, 100)
		);
	if (m_axisEnable & 4)
		primitiveRenderer->drawLine(
			Vector4(0.0f, 0.0f, -c_infinite, 1.0f),
			Vector4(0.0f, 0.0f, c_infinite, 1.0f),
			1.0f,
			Color4ub(0, 0, 255, 100)
		);
	primitiveRenderer->popDepthEnable();

	primitiveRenderer->pushDepthEnable(false);

	// Guide fill squares.
	// XY
	primitiveRenderer->drawSolidQuad(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(squareLength, 0.0f, 0.0f, 1.0f),
		Vector4(squareLength, squareLength, 0.0f, 1.0f),
		Vector4(0.0f, squareLength, 0.0f, 1.0f),
		Color4ub(255, 255, 0, 70)
	);
	// XZ
	primitiveRenderer->drawSolidQuad(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(squareLength, 0.0f, 0.0f, 1.0f),
		Vector4(squareLength, 0.0f, squareLength, 1.0f),
		Vector4(0.0f, 0.0f, squareLength, 1.0f),
		Color4ub(255, 255, 0, 70)
	);
	// YZ
	primitiveRenderer->drawSolidQuad(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, squareLength, 0.0f, 1.0f),
		Vector4(0.0f, squareLength, squareLength, 1.0f),
		Vector4(0.0f, 0.0f, squareLength, 1.0f),
		Color4ub(255, 255, 0, 70)
	);

	// Guide square lines.
	// XY
	primitiveRenderer->drawLine(Vector4(squareLength, 0.0f, 0.0f, 1.0f), Vector4(squareLength, squareLength, 0.0f, 1.0f), ((m_axisEnable & (1 | 2)) == (1 | 2)) ? 3.0f : 1.0f, Color4ub(255, 0, 0, 255));
	primitiveRenderer->drawLine(Vector4(squareLength, 0.0f, 0.0f, 1.0f), Vector4(squareLength, 0.0f, squareLength, 1.0f), ((m_axisEnable & (1 | 4)) == (1 | 4)) ? 3.0f : 1.0f, Color4ub(255, 0, 0, 255));
	// XZ
	primitiveRenderer->drawLine(Vector4(0.0f, squareLength, 0.0f, 1.0f), Vector4(squareLength, squareLength, 0.0f, 1.0f), ((m_axisEnable & (1 | 2)) == (1 | 2)) ? 3.0f : 1.0f, Color4ub(0, 255, 0, 255));
	primitiveRenderer->drawLine(Vector4(0.0f, squareLength, 0.0f, 1.0f), Vector4(0.0f, squareLength, squareLength, 1.0f), ((m_axisEnable & (2 | 4)) == (2 | 4)) ? 3.0f : 1.0f, Color4ub(0, 255, 0, 255));
	// YZ
	primitiveRenderer->drawLine(Vector4(0.0f, 0.0f, squareLength, 1.0f), Vector4(squareLength, 0.0f, squareLength, 1.0f), ((m_axisEnable & (1 | 4)) == (1 | 4)) ? 3.0f : 1.0f, Color4ub(0, 0, 255, 255));
	primitiveRenderer->drawLine(Vector4(0.0f, 0.0f, squareLength, 1.0f), Vector4(0.0f, squareLength, squareLength, 1.0f), ((m_axisEnable & (2 | 4)) == (2 | 4)) ? 3.0f : 1.0f, Color4ub(0, 0, 255, 255));

	// Guide axis lines.
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(axisLength, 0.0f, 0.0f, 1.0f),
		(m_axisEnable & 1) ? 3.0f : 1.0f,
		Color4ub(255, 0, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, axisLength, 0.0f, 1.0f),
		(m_axisEnable & 2) ? 3.0f : 1.0f,
		Color4ub(0, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, 0.0f, axisLength, 1.0f),
		(m_axisEnable & 4) ? 3.0f : 1.0f,
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

	primitiveRenderer->popDepthEnable();

	primitiveRenderer->popWorld();
}

	}
}
