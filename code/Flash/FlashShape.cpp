#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashShapeInstance.h"
#include "Flash/Path.h"
#include "Flash/SwfMembers.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashShape", 0, FlashShape, FlashCharacter)

FlashShape::FlashShape()
{
}

FlashShape::FlashShape(uint16_t id)
:	FlashCharacter(id)
{
	m_shapeBounds.mn.x = m_shapeBounds.mx.x =
	m_shapeBounds.mn.y = m_shapeBounds.mx.y = 0.0f;
}

bool FlashShape::create(const Aabb2& shapeBounds, const SwfShape* shape, const SwfStyles* styles)
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

	for (uint16_t i = 0; i < shape->numShapeRecords; ++i)
	{
		SwfShapeRecord* shapeRecord = shape->shapeRecords[i];
		if (shapeRecord->edgeFlag && shapeRecord->edge.straightFlag)
		{
			const SwfStraightEdgeRecord& s = shapeRecord->edge.straightEdge;
			if (s.generalLineFlag)
				path.lineTo(float(s.deltaX), float(s.deltaY), Path::CmRelative);
			else
			{
				if (!s.vertLineFlag)
					path.lineTo(float(s.deltaX), 0.0f, Path::CmRelative);
				else
					path.lineTo(0.0f, float(s.deltaY), Path::CmRelative);
			}
		}
		else if (shapeRecord->edgeFlag && !shapeRecord->edge.straightFlag)
		{
			const SwfCurvedEdgeRecord& c = shapeRecord->edge.curvedEdge;
			path.quadraticTo(
				float(c.controlDeltaX),
				float(c.controlDeltaY),
				float(c.controlDeltaX + c.anchorDeltaX),
				float(c.controlDeltaY + c.anchorDeltaY),
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
					float(s.moveDeltaX),
					float(s.moveDeltaY),
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

bool FlashShape::create(const SwfShape* shape)
{
	uint16_t fillStyle0 = 0;
	uint16_t fillStyle1 = 0;
	uint16_t lineStyle = 0;

	Path path;

	for (uint16_t i = 0; i < shape->numShapeRecords; ++i)
	{
		SwfShapeRecord* shapeRecord = shape->shapeRecords[i];
		if (shapeRecord->edgeFlag && shapeRecord->edge.straightFlag)
		{
			const SwfStraightEdgeRecord& s = shapeRecord->edge.straightEdge;
			if (s.generalLineFlag)
				path.lineTo(float(s.deltaX), float(s.deltaY), Path::CmRelative);
			else
			{
				if (!s.vertLineFlag)
					path.lineTo(float(s.deltaX), 0.0f, Path::CmRelative);
				else
					path.lineTo(0.0f, float(s.deltaY), Path::CmRelative);
			}
		}
		else if (shapeRecord->edgeFlag && !shapeRecord->edge.straightFlag)
		{
			const SwfCurvedEdgeRecord& c = shapeRecord->edge.curvedEdge;
			path.quadraticTo(
				float(c.controlDeltaX),
				float(c.controlDeltaY),
				float(c.controlDeltaX + c.anchorDeltaX),
				float(c.controlDeltaY + c.anchorDeltaY),
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
					float(s.moveDeltaX),
					float(s.moveDeltaY),
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
	
	m_shapeBounds = path.getBounds();
	m_paths.push_back(path);

	return true;
}

bool FlashShape::create(uint16_t fillBitmap, float width, float height)
{
	m_fillStyles.resize(1);
	m_fillStyles[0].create(fillBitmap, Matrix33(
		20.0f, 0.0f, 0.0f,
		0.0f, 20.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	));

	Path path;
	path.moveTo(0.0f, 0.0f, Path::CmAbsolute);
	path.lineTo(width, 0.0f, Path::CmAbsolute);
	path.lineTo(width, height, Path::CmAbsolute);
	path.lineTo(0.0f, height, Path::CmAbsolute);
	path.lineTo(0.0f, 0.0f, Path::CmAbsolute);
	path.end(1, 1, 0);

	m_shapeBounds = path.getBounds();
	m_paths.push_back(path);

	return true;
}

Ref< FlashCharacterInstance > FlashShape::createInstance(
	ActionContext* context,
	FlashCharacterInstance* parent,
	const std::string& name,
	const ActionObject* initObject,
	const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
) const
{
	return new FlashShapeInstance(context, parent, this);
}

void FlashShape::serialize(ISerializer& s)
{
	FlashCharacter::serialize(s);

	s >> MemberAabb2(L"shapeBounds", m_shapeBounds);
	s >> MemberStlList< Path, MemberComposite< Path > >(L"paths", m_paths);
	s >> MemberAlignedVector< FlashFillStyle, MemberComposite< FlashFillStyle > >(L"fillStyles", m_fillStyles);
	s >> MemberAlignedVector< FlashLineStyle, MemberComposite< FlashLineStyle > >(L"lineStyles", m_lineStyles);
}

	}
}
