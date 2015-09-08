#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Spark/Editor/Shape/TesselatorVisitor.h"
#include "Spark/Editor/Shape/Shape.h"
#include "Spark/Editor/Shape/PathShape.h"
#include "Spark/Editor/Shape/Path.h"
#include "Spark/Editor/Shape/Style.h"

namespace traktor
{
	namespace spark
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.TesselatorVisitor", TesselatorVisitor, ShapeVisitor)

TesselatorVisitor::TesselatorVisitor(Listener& listener, const Style* style)
:	m_listener(listener)
,	m_style(style)
{
	m_transform.push(Matrix33::identity());
}
	
void TesselatorVisitor::enter(Shape* shape)
{
	T_ASSERT (shape);

	if (shape->getStyle())
		m_style = shape->getStyle();

	m_transform.push(shape->getTransform() * m_transform.top());

	PathShape* pathShape = dynamic_type_cast< PathShape* >(shape);
	if (pathShape)
		freezePath(pathShape->getPath());
}

void TesselatorVisitor::leave(Shape* shape)
{
	m_transform.pop();
}

void TesselatorVisitor::freezePath(const Path& path)
{
	AlignedVector< Vector2 > shape;

	const std::vector< SubPath >& subPaths = path.getSubPaths();
	for (std::vector< SubPath >::const_iterator i = subPaths.begin(); i != subPaths.end(); ++i)
	{
		switch (i->type)
		{
		case SptLinear:
			freezeLinearSubPath(*i, shape);
			break;

		case SptCubic:
			freezeCubicSubPath(*i, shape);
			break;
		}
	}

	if (!shape.empty())
		m_listener.drawShape(m_style, shape);
}

void TesselatorVisitor::freezeLinearSubPath(const SubPath& subPath, AlignedVector< Vector2 >& outShape)
{
	T_ASSERT (!subPath.points.empty());
	for (std::vector< Vector2 >::const_iterator i = subPath.points.begin(); i != subPath.points.end(); ++i)
		outShape.push_back(*i);
}

namespace
{

	Vector2 evalCubic(
		float t,
		const Vector2& cp0,
		const Vector2& cp1,
		const Vector2& cp2,
		const Vector2& cp3
	)
	{
		float it1 = 1.0f - t;
		float it2 = it1 * it1;
		float it3 = it2 * it1;

		float t2 = t * t;
		float t3 = t2 * t;

		return
			cp0 * it3 +
			3.0f * cp1 * t * it2 +
			3.0f * cp2 * t2 * it1 +
			cp3 * t3;
	}

}

void TesselatorVisitor::freezeCubicSubPath(const SubPath& subPath, AlignedVector< Vector2 >& outShape)
{
	T_ASSERT (!subPath.points.empty());

	const std::vector< Vector2 >& cp = subPath.points;

	for (int i = 0; i < int(cp.size() - 1); i += 3)
	{
		const Vector2& cp0 = cp[i];
		const Vector2& cp1 = cp[i + 1];
		const Vector2& cp2 = cp[i + 2];
		const Vector2& cp3 = cp[i + 3];

		float t = 0.0f;
		float s = 0.2f;
		while (t < 1.0f)
		{
			Vector2 ps = evalCubic(t - s / 2.0f, cp0, cp1, cp2, cp3);
			Vector2 pm = evalCubic(t           , cp0, cp1, cp2, cp3);
			Vector2 pe = evalCubic(t + s / 2.0f, cp0, cp1, cp2, cp3);

			float err = (pm - (ps + pe) / 2.0f).length();
			if (err < 0.2f || s < 0.01f)
			{
				outShape.push_back(pm);
				t += s; s = 0.2f;
			}
			else
				s /= 2.0f;
		}
	}
}
	
	}
}
