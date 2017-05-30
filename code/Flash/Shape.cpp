/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/Shape.h"
#include "Flash/ShapeInstance.h"
#include "Flash/Path.h"
#include "Flash/SwfMembers.h"
#include "Flash/Triangulator.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

class MemberTriangle : public MemberComplex
{
public:
	MemberTriangle(const wchar_t* const name, Triangle& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		s >> MemberStaticArray< Vector2, 3 >(L"v", m_ref.v);
		s >> Member< uint8_t >(L"type", m_ref.type);
		s >> Member< uint16_t >(L"fillStyle", m_ref.fillStyle);
	}

private:
	Triangle& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.Shape", 0, Shape, Character)

Shape::Shape()
{
}

Shape::Shape(uint16_t id)
:	Character(id)
{
}

bool Shape::create(const Aabb2& shapeBounds, const SwfShape* shape, const SwfStyles* styles)
{
	uint16_t fillStyle0 = 0;
	uint16_t fillStyle1 = 0;
	uint16_t lineStyle = 0;
	uint32_t fillStyleBase = 0;
	uint32_t lineStyleBase = 0;

	m_fillStyles.resize(styles->numFillStyles);
	for (uint32_t i = 0; i < styles->numFillStyles; ++i)
		m_fillStyles[i].create(styles->fillStyles[i]);

	m_lineStyles.resize(styles->numLineStyles);
	for (uint32_t i = 0; i < styles->numLineStyles; ++i)
		m_lineStyles[i].create(styles->lineStyles[i]);

	Path path;
	for (SwfShapeRecord* shapeRecord = shape->shapeRecords; shapeRecord; shapeRecord = shapeRecord->next)
	{
		if (shapeRecord->edgeFlag && shapeRecord->edge.straightFlag)
		{
			const SwfStraightEdgeRecord& s = shapeRecord->edge.straightEdge;
			if (s.generalLineFlag)
				path.lineTo(s.deltaX, s.deltaY, Path::CmRelative);
			else
			{
				if (!s.vertLineFlag)
					path.lineTo(s.deltaX, 0, Path::CmRelative);
				else
					path.lineTo(0, s.deltaY, Path::CmRelative);
			}
		}
		else if (shapeRecord->edgeFlag && !shapeRecord->edge.straightFlag)
		{
			const SwfCurvedEdgeRecord& c = shapeRecord->edge.curvedEdge;
			path.quadraticTo(
				c.controlDeltaX,
				c.controlDeltaY,
				c.controlDeltaX + c.anchorDeltaX,
				c.controlDeltaY + c.anchorDeltaY,
				Path::CmRelative
			);
		}
		else if (!shapeRecord->edgeFlag)
		{
			// Whenever a style records appear we close the current sub path.
			path.end(
				fillStyle0 ? fillStyle0 + fillStyleBase : 0,
				fillStyle1 ? fillStyle1 + fillStyleBase : 0,
				lineStyle ? lineStyle + lineStyleBase : 0
			);

			const SwfStyleRecord& s = shapeRecord->style;
			if (s.stateMoveTo)
			{
				path.moveTo(
					s.moveDeltaX,
					s.moveDeltaY,
					Path::CmAbsolute
				);
			}

			if (s.stateNewStyles)
			{
				T_ASSERT (s.newStyles);

				fillStyleBase = uint32_t(m_fillStyles.size());
				lineStyleBase = uint32_t(m_lineStyles.size());

				if (s.newStyles->numFillStyles > 0)
				{
					m_fillStyles.resize(fillStyleBase + s.newStyles->numFillStyles);
					for (int j = 0; j < s.newStyles->numFillStyles; ++j)
					{
						if (!m_fillStyles[fillStyleBase + j].create(s.newStyles->fillStyles[j]))
							return false;
					}
				}

				if (s.newStyles->numLineStyles > 0)
				{
					m_lineStyles.resize(lineStyleBase + s.newStyles->numLineStyles);
					for (int j = 0; j < s.newStyles->numLineStyles; ++j)
					{
						if (!m_lineStyles[lineStyleBase + j].create(s.newStyles->lineStyles[j]))
							return false;
					}
				}

				fillStyle0 =
				fillStyle1 =
				lineStyle = 0;

				m_paths.push_back(path);
				path.reset();
			}

			if (s.stateFillStyle0)
				fillStyle0 = s.fillStyle0;
			if (s.stateFillStyle1)
				fillStyle1 = s.fillStyle1;
			if (s.stateLineStyle)
				lineStyle = s.lineStyle;
		}
	}

	path.end(
		fillStyle0 ? fillStyle0 + fillStyleBase : 0,
		fillStyle1 ? fillStyle1 + fillStyleBase : 0,
		lineStyle ? lineStyle + lineStyleBase : 0
	);

	m_paths.push_back(path);
	m_shapeBounds = shapeBounds;
	return true;
}

bool Shape::create(const SwfShape* shape)
{
	uint16_t fillStyle0 = 0;
	uint16_t fillStyle1 = 0;
	uint16_t lineStyle = 0;

	Path path;
	for (SwfShapeRecord* shapeRecord = shape->shapeRecords; shapeRecord; shapeRecord = shapeRecord->next)
	{
		if (shapeRecord->edgeFlag && shapeRecord->edge.straightFlag)
		{
			const SwfStraightEdgeRecord& s = shapeRecord->edge.straightEdge;
			if (s.generalLineFlag)
				path.lineTo(s.deltaX, s.deltaY, Path::CmRelative);
			else
			{
				if (!s.vertLineFlag)
					path.lineTo(s.deltaX, 0, Path::CmRelative);
				else
					path.lineTo(0, s.deltaY, Path::CmRelative);
			}
		}
		else if (shapeRecord->edgeFlag && !shapeRecord->edge.straightFlag)
		{
			const SwfCurvedEdgeRecord& c = shapeRecord->edge.curvedEdge;
			path.quadraticTo(
				c.controlDeltaX,
				c.controlDeltaY,
				c.controlDeltaX + c.anchorDeltaX,
				c.controlDeltaY + c.anchorDeltaY,
				Path::CmRelative
			);
		}
		else if (!shapeRecord->edgeFlag)
		{
			// Whenever a style records appear we close the current sub path.
			path.end(fillStyle0, fillStyle1, lineStyle);

			const SwfStyleRecord& s = shapeRecord->style;
			if (s.stateMoveTo)
			{
				path.moveTo(
					s.moveDeltaX,
					s.moveDeltaY,
					Path::CmAbsolute
				);
			}

			if (s.stateNewStyles)
			{
				fillStyle0 =
				fillStyle1 =
				lineStyle = 0;

				m_paths.push_back(path);
				path.reset();
			}

			if (s.stateFillStyle0)
				fillStyle0 = s.fillStyle0;
			if (s.stateFillStyle1)
				fillStyle1 = s.fillStyle1;
			if (s.stateLineStyle)
				lineStyle = s.lineStyle;
		}
	}

	path.end(fillStyle0, fillStyle1, lineStyle);

	m_paths.push_back(path);
	m_shapeBounds = path.getBounds();
	return true;
}

bool Shape::create(uint16_t fillBitmap, int32_t width, int32_t height)
{
	m_fillStyles.resize(1);
	m_fillStyles[0].create(fillBitmap, Matrix33(
		20.0f, 0.0f, 0.0f,
		0.0f, 20.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	), true);

	Path path;
	path.moveTo(0, 0, Path::CmAbsolute);
	path.lineTo(width, 0, Path::CmAbsolute);
	path.lineTo(width, height, Path::CmAbsolute);
	path.lineTo(0, height, Path::CmAbsolute);
	path.lineTo(0, 0, Path::CmAbsolute);
	path.end(1, 1, 0);

	m_paths.push_back(path);
	m_shapeBounds = path.getBounds();
	return true;
}

void Shape::merge(const Shape& shape, const Matrix33& transform, const ColorTransform& cxform)
{
	uint32_t fillStyleBase = uint32_t(m_fillStyles.size());
	uint32_t lineStyleBase = uint32_t(m_lineStyles.size());

	// Transform fill styles.
	for (uint32_t i = 0; i < shape.getFillStyles().size(); ++i)
	{
		FillStyle fillStyle = shape.getFillStyles()[i];
		fillStyle.transform(transform, cxform);
		m_fillStyles.push_back(fillStyle);
	}

	// Transform line styles.
	for (uint32_t i = 0; i < shape.getLineStyles().size(); ++i)
	{
		LineStyle lineStyle = shape.getLineStyles()[i];
		lineStyle.transform(cxform);
		m_lineStyles.push_back(lineStyle);
	}

	// Transform paths and modify styles.
	for (AlignedVector< Path >::const_iterator i = shape.getPaths().begin(); i != shape.getPaths().end(); ++i)
	{
		AlignedVector< SubPath > subPaths = i->getSubPaths();
		for (AlignedVector< SubPath >::iterator j = subPaths.begin(); j != subPaths.end(); ++j)
		{
			if (j->fillStyle0)
				j->fillStyle0 = fillStyleBase + j->fillStyle0;
			if (j->fillStyle1)
				j->fillStyle1 = fillStyleBase + j->fillStyle1;
			if (j->lineStyle)
				j->lineStyle = lineStyleBase + j->lineStyle;
		}
		m_paths.push_back(Path(transform, i->getPoints(), subPaths));
	}

	// Expand our bounds with transformed shape's bound.
	const Aabb2& shapeBounds = shape.getShapeBounds();
	Vector2 shapeExtents[4];
	shapeBounds.getExtents(shapeExtents);
	for (uint32_t i = 0; i < sizeof_array(shapeExtents); ++i)
		m_shapeBounds.contain(transform * shapeExtents[i]);
}

void Shape::triangulate(bool oddEven)
{
	AlignedVector< Segment > segments;
	Triangulator triangulator;
	Segment s;

	m_triangles.resize(0);

	for (AlignedVector< Path >::const_iterator i = m_paths.begin(); i != m_paths.end(); ++i)
	{
		const AlignedVector< Vector2 >& points = i->getPoints();
		const AlignedVector< SubPath >& subPaths = i->getSubPaths();

		std::set< uint16_t > fillStyles;
		for (uint32_t j = 0; j < subPaths.size(); ++j)
		{
			const SubPath& sp = subPaths[j];
			if (sp.fillStyle0)
				fillStyles.insert(sp.fillStyle0);
			if (sp.fillStyle1)
				fillStyles.insert(sp.fillStyle1);
		}

		for (std::set< uint16_t >::const_iterator ii = fillStyles.begin(); ii != fillStyles.end(); ++ii)
		{
			for (uint32_t j = 0; j < subPaths.size(); ++j)
			{
				const SubPath& sp = subPaths[j];
				if (sp.fillStyle0 != *ii && sp.fillStyle1 != *ii)
					continue;

				for (AlignedVector< SubPathSegment >::const_iterator k = sp.segments.begin(); k != sp.segments.end(); ++k)
				{
					switch (k->type)
					{
					case SpgtLinear:
						{
							s.v[0] = points[k->pointsOffset];
							s.v[1] = points[k->pointsOffset + 1];
							s.curve = false;
							s.fillStyle0 = sp.fillStyle0;
							s.fillStyle1 = sp.fillStyle1;
							s.lineStyle = sp.lineStyle;
							segments.push_back(s);
						}
						break;

					case SpgtQuadratic:
						{
							s.v[0] = points[k->pointsOffset];
							s.v[1] = points[k->pointsOffset + 2];
							s.c = points[k->pointsOffset + 1];
							s.curve = true;
							s.fillStyle0 = sp.fillStyle0;
							s.fillStyle1 = sp.fillStyle1;
							s.lineStyle = sp.lineStyle;
							segments.push_back(s);
						}
						break;

					default:
						break;
					}
				}
			}

			if (!segments.empty())
			{
				uint32_t from = m_triangles.size();

				triangulator.triangulate(segments, *ii, oddEven, m_triangles);
				segments.resize(0);

				uint32_t to = m_triangles.size();

				// Transform each new triangle with path's transform.
				for (uint32_t ti = from; ti < to; ++ti)
				{
					m_triangles[ti].v[0] = i->getTransform() * m_triangles[ti].v[0];
					m_triangles[ti].v[1] = i->getTransform() * m_triangles[ti].v[1];
					m_triangles[ti].v[2] = i->getTransform() * m_triangles[ti].v[2];
				}
			}
		}
	}
}

void Shape::discardPaths()
{
	m_paths.clear();
}

Ref< CharacterInstance > Shape::createInstance(
	ActionContext* context,
	Dictionary* dictionary,
	CharacterInstance* parent,
	const std::string& name,
	const Matrix33& transform,
	const ActionObject* initObject,
	const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
) const
{
	return new ShapeInstance(context, dictionary, parent, this);
}

void Shape::serialize(ISerializer& s)
{
	Character::serialize(s);

	s >> MemberAabb2(L"shapeBounds", m_shapeBounds);
	s >> MemberAlignedVector< Path, MemberComposite< Path > >(L"paths", m_paths);
	s >> MemberAlignedVector< FillStyle, MemberComposite< FillStyle > >(L"fillStyles", m_fillStyles);
	s >> MemberAlignedVector< LineStyle, MemberComposite< LineStyle > >(L"lineStyles", m_lineStyles);
	s >> MemberAlignedVector< Triangle, MemberTriangle >(L"triangles", m_triangles);
}

	}
}
