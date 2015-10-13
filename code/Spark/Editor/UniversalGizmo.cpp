#include "Core/Log/Log.h"
#include "Render/PrimitiveRenderer.h"
#include "Spark/CharacterInstance.h"
#include "Spark/Editor/CharacterAdapter.h"
#include "Spark/Editor/Context.h"
#include "Spark/Editor/UniversalGizmo.h"
#include "Ui/Widget.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

const Vector2 c_hitBoxSize(20.0f, 20.0f);
const Vector2 c_centerHitBoxSize(40.0f, 40.0f);

Matrix33 orthogonalize(const Matrix33& m)
{
	Vector2 A(m.e11, m.e21);
	Vector2 B = -A.perpendicular().normalized() * Vector2(m.e12, m.e22).length();
	return Matrix33(
		A.x, B.x, m.e13,
		A.y, B.y, m.e23,
		0.0f, 0.0f, 1.0f
	);
}

int32_t pointInBoxes(const Vector2& point, const Vector2* boxCenters, int32_t numBoxCenters, const Vector2& boxSize)
{
	for (int32_t i = 0; i < numBoxCenters; ++i)
	{
		if (Aabb2(boxCenters[i] - boxSize / 2.0f, boxCenters[i] + boxSize / 2.0f).inside(point))
			return i;
	}
	return -1;
}

void drawWireCross(render::PrimitiveRenderer* primitiveRenderer, const Vector2& center, float r, const Color4ub& color)
{
	primitiveRenderer->drawLine(
		Vector4(center.x - r, center.y, 1.0f, 1.0f),
		Vector4(center.x + r, center.y, 1.0f, 1.0f),
		color
	);
	primitiveRenderer->drawLine(
		Vector4(center.x, center.y - r, 1.0f, 1.0f),
		Vector4(center.x, center.y + r, 1.0f, 1.0f),
		color
	);
}

void drawWireQuad(render::PrimitiveRenderer* primitiveRenderer, const Vector2* e, const Color4ub& color)
{
	primitiveRenderer->drawLine(
		Vector4(e[0].x, e[0].y, 1.0f, 1.0f),
		Vector4(e[1].x, e[1].y, 1.0f, 1.0f),
		color
	);
	primitiveRenderer->drawLine(
		Vector4(e[1].x, e[1].y, 1.0f, 1.0f),
		Vector4(e[2].x, e[2].y, 1.0f, 1.0f),
		color
	);
	primitiveRenderer->drawLine(
		Vector4(e[2].x, e[2].y, 1.0f, 1.0f),
		Vector4(e[3].x, e[3].y, 1.0f, 1.0f),
		color
	);
	primitiveRenderer->drawLine(
		Vector4(e[3].x, e[3].y, 1.0f, 1.0f),
		Vector4(e[0].x, e[0].y, 1.0f, 1.0f),
		color
	);
}

void drawWireBoxes(render::PrimitiveRenderer* primitiveRenderer, const Matrix33& transform, const Vector2* boxCenters, int32_t numBoxCenters, const Vector2& boxSize, const Color4ub& color)
{
	for (int32_t i = 0; i < numBoxCenters; ++i)
	{
		Vector2 center = transform * boxCenters[i];
		Vector2 e[] =
		{
			center + Vector2(-boxSize.x / 2.0f, -boxSize.y / 2.0f),
			center + Vector2( boxSize.x / 2.0f, -boxSize.y / 2.0f),
			center + Vector2( boxSize.x / 2.0f,  boxSize.y / 2.0f),
			center + Vector2(-boxSize.x / 2.0f,  boxSize.y / 2.0f)
		};
		drawWireQuad(primitiveRenderer, e, color);
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.UniversalGizmo", UniversalGizmo, IGizmo)

UniversalGizmo::UniversalGizmo(Context* context)
:	m_context(context)
,	m_mode(EmNone)
{
}

void UniversalGizmo::attach(CharacterAdapter* adapter)
{
	CharacterInstance* character = adapter->getCharacterInstance();
	T_FATAL_ASSERT (character);

	Aabb2 bounds = character->getBounds();

	m_center = bounds.getCenter();

	m_scaleCorners[0] = bounds.mn;
	m_scaleCorners[1] = Vector2(bounds.mx.x, bounds.mn.y);
	m_scaleCorners[2] = bounds.mx;
	m_scaleCorners[3] = Vector2(bounds.mn.x, bounds.mx.y);

	m_scaleEdges[0] = (m_scaleCorners[0] + m_scaleCorners[1]) / 2.0f;
	m_scaleEdges[1] = (m_scaleCorners[1] + m_scaleCorners[2]) / 2.0f;
	m_scaleEdges[2] = (m_scaleCorners[2] + m_scaleCorners[3]) / 2.0f;
	m_scaleEdges[3] = (m_scaleCorners[3] + m_scaleCorners[0]) / 2.0f;

	m_rotateCorners[0] = m_scaleCorners[0] + Vector2(-c_hitBoxSize.x, -c_hitBoxSize.y);
	m_rotateCorners[1] = m_scaleCorners[1] + Vector2( c_hitBoxSize.x, -c_hitBoxSize.y);
	m_rotateCorners[2] = m_scaleCorners[2] + Vector2( c_hitBoxSize.x,  c_hitBoxSize.y);
	m_rotateCorners[3] = m_scaleCorners[3] + Vector2(-c_hitBoxSize.x,  c_hitBoxSize.y);
}

void UniversalGizmo::detach(CharacterAdapter* adapter)
{
}

void UniversalGizmo::mouseDown(ui::Widget* widget, CharacterAdapter* adapter, const Vector2& position)
{
	int32_t hit;

	CharacterInstance* character = adapter->getCharacterInstance();
	T_FATAL_ASSERT (character);

	Vector2 localPosition = character->getFullTransform().inverse() * position;

	if ((hit = pointInBoxes(localPosition, &m_center, 1, c_centerHitBoxSize)) >= 0)
	{
		m_mode = EmTranslate;
		m_lastPosition = position;
	}
	else if ((hit = pointInBoxes(localPosition, m_scaleCorners, sizeof_array(m_scaleCorners), c_hitBoxSize)) >= 0)
	{
		m_mode = EmScale;
		m_lastPosition = position;
	}
	else if ((hit = pointInBoxes(localPosition, m_scaleEdges, sizeof_array(m_scaleEdges), c_hitBoxSize)) >= 0)
	{
		m_mode = EmScale;
		m_lastPosition = position;
	}
	else if ((hit = pointInBoxes(localPosition, m_rotateCorners, sizeof_array(m_rotateCorners), c_hitBoxSize)) >= 0)
	{
		m_mode = EmRotate;
		m_lastPosition = position;
	}
}

void UniversalGizmo::mouseUp(ui::Widget* widget, CharacterAdapter* adapter, const Vector2& position)
{
	m_mode = EmNone;
}

void UniversalGizmo::mouseMove(ui::Widget* widget, CharacterAdapter* adapter, const Vector2& position)
{
	CharacterInstance* character = adapter->getCharacterInstance();
	T_FATAL_ASSERT (character);

	if (m_mode == EmTranslate)
	{
		Vector2 delta = position - m_lastPosition;
		adapter->setTransform(orthogonalize(translate(delta.x, delta.y) * adapter->getTransform()));
		m_lastPosition = position;
	}
	else if (m_mode == EmScale)
	{
		Matrix33 Tinverse = character->getFullTransform().inverse();

		Vector2 delta = Tinverse * position - Tinverse * m_lastPosition;
		Vector2 v = m_scaleCorners[2] - m_scaleCorners[0];
		Vector2 f = (v + delta * 2.0f) / v;

		adapter->setTransform(orthogonalize(adapter->getTransform() * scale(f.x, f.y)));

		attach(adapter);

		m_lastPosition = position;
	}
	else if (m_mode == EmRotate)
	{
		Vector2 center = character->getFullTransform() * m_center;

		Vector2 A = (m_lastPosition - center).normalized();
		Vector2 B = (position - center).normalized();

		float a = dot(A, B);
		float b = dot(A, B.perpendicular());

		float angle = std::atan2(b, a);
		adapter->setTransform(orthogonalize(adapter->getTransform() * rotate(angle)));

		m_lastPosition = position;
	}
	else
	{
		Vector2 localPosition = character->getFullTransform().inverse() * position;
		int32_t hit;

		if ((hit = pointInBoxes(localPosition, &m_center, 1, c_centerHitBoxSize)) >= 0)
		{
			widget->setCursor(ui::CrSizing);
		}
		else if ((hit = pointInBoxes(localPosition, m_scaleCorners, sizeof_array(m_scaleCorners), c_hitBoxSize)) >= 0)
		{
			widget->setCursor(ui::CrSizing);
		}
		else if ((hit = pointInBoxes(localPosition, m_scaleEdges, sizeof_array(m_scaleEdges), c_hitBoxSize)) >= 0)
		{
			if (hit == 0 || hit == 2)
				widget->setCursor(ui::CrSizeNS);
			else
				widget->setCursor(ui::CrSizeWE);
		}
		else if ((hit = pointInBoxes(localPosition, m_rotateCorners, sizeof_array(m_rotateCorners), c_hitBoxSize)) >= 0)
		{
			widget->setCursor(ui::CrHand);
		}
		else
			widget->setCursor(ui::CrArrow);
	}
}

void UniversalGizmo::paint(CharacterAdapter* adapter, render::PrimitiveRenderer* primitiveRenderer)
{
	CharacterInstance* character = adapter->getCharacterInstance();
	T_FATAL_ASSERT (character);

	Matrix33 T = character->getFullTransform();
	Aabb2 bounds = character->getBounds();

	Vector2 e[] =
	{
		T * bounds.mn,
		T * Vector2(bounds.mx.x, bounds.mn.y),
		T * bounds.mx,
		T * Vector2(bounds.mn.x, bounds.mx.y)
	};

	drawWireCross(primitiveRenderer, Vector2(T.e13, T.e23), 20.0f, Color4ub(0, 255, 0, 255));
	drawWireQuad(primitiveRenderer, e, Color4ub(255, 255, 0, 255));

	drawWireBoxes(primitiveRenderer, T, &m_center, 1, c_centerHitBoxSize, Color4ub(255, 0, 0, 255));
	drawWireBoxes(primitiveRenderer, T, m_scaleCorners, sizeof_array(m_scaleCorners), c_hitBoxSize, Color4ub(255, 0, 0, 255));
	drawWireBoxes(primitiveRenderer, T, m_scaleEdges, sizeof_array(m_scaleEdges), c_hitBoxSize, Color4ub(255, 0, 0, 255));
	//drawWireBoxes(primitiveRenderer, T, m_rotateCorners, sizeof_array(m_rotateCorners), c_hitBoxSize, Color4ub(255, 0, 0, 255));
}

	}
}
