/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <functional>
#include "Drawing/Image.h"
#include "Drawing/Raster.h"
#include "Svg/Document.h"
#include "Svg/Gradient.h"
#include "Svg/IShapeVisitor.h"
#include "Svg/PathShape.h"
#include "Svg/Rasterizer.h"
#include "Svg/Style.h"

namespace traktor::svg
{
	namespace
	{

class LambdaShapeVisitor : public IShapeVisitor
{
public:
	explicit LambdaShapeVisitor(const std::function< bool(Shape*) >& enter, const std::function< void(Shape*) >& leave)
	:	m_enter(enter)
	,	m_leave(leave)
	{
	}

	virtual bool enter(Shape* shape) override final { return m_enter(shape); }

	virtual void leave(Shape* shape) override final { m_leave(shape); }

private:
	std::function< bool(Shape*) > m_enter;
	std::function< void(Shape*) > m_leave;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.Rasterizer", Rasterizer, Object)

bool Rasterizer::raster(const Document* document, drawing::Image* image) const
{
	const Aabb2& viewBox = document->getViewBox();

	const float sx = image->getWidth() / viewBox.getSize().x;
	const float sy = image->getHeight() / viewBox.getSize().y;

	const Matrix33 Mview = scale(sx, sy);

	drawing::Raster raster(image);

	LambdaShapeVisitor visitor(
		[&](Shape* shape) -> bool
		{
			if (auto pathShape = dynamic_type_cast< const PathShape* >(shape))
			{
				const Matrix33 Mshape = Mview * shape->getGlobalTransform();

				raster.clear();
				for (const auto& subPath : pathShape->getPath().getSubPaths())
				{
					const auto& points = subPath.points;

					switch (subPath.type)
					{
					case SubPathType::Linear:
						raster.moveTo(Mshape * points[0]);
						for (size_t i = 1; i < points.size(); ++i)
							raster.lineTo(Mshape * points[i]);
						break;

					case SubPathType::Quadric:
						raster.moveTo(Mshape * points[0]);
						for (size_t i = 1; i < points.size(); i += 2)
							raster.quadricTo(
								Mshape * points[i],
								Mshape * points[i + 1]
							);
						break;

					case SubPathType::Cubic:
						raster.moveTo(Mshape * points[0]);
						for (size_t i = 1; i < points.size(); i += 3)
							raster.cubicTo(
								Mshape * points[i],
								Mshape * points[i + 1],
								Mshape * points[i + 2]
							);
						break;

					default:
						break;
					}

					if (subPath.closed)
					{
						raster.lineTo(Mshape * subPath.origin);
						raster.close();
					}
				}

				const Style* style = pathShape->getStyle();
				if (style)
				{
					const Color4f opacity(1.0f, 1.0f, 1.0f, style->getOpacity());
					if (style->getFillEnable())
					{
						const Gradient* fillGradient = style->getFillGradient();
						if (fillGradient && fillGradient->getStopCount() >= 2)
						{
							const Vector2& v1 = fillGradient->getBounds().mn;
							const Vector2& v2 = fillGradient->getBounds().mx;

							const Vector2 v = v2 - v1;
							const float dv = v.length();
							const float a = std::atan2(v.y, v.x);

							const Matrix33 Mt = translate(-v1);
							const Matrix33 Ma = rotate(-a);
							const Matrix33 Ms = scale(1.0f / dv, 1.0f);
							const Matrix33 Mg = Ms * Ma * Mt * (Mshape * fillGradient->getTransform()).inverse();

							AlignedVector< std::pair< Color4f, float > > stops;
							for (const auto & st : fillGradient->getStops())
							{
								stops.push_back({
									st.color * opacity,
									st.offset
								});
							}
							const int32_t st = raster.defineLinearGradientStyle(Mg, stops);
							raster.fill(-1, st, drawing::Raster::FillRule::OddEven);
						}
						else
						{
							const int32_t st = raster.defineSolidStyle(style->getFill() * opacity);
							raster.fill(-1, st, drawing::Raster::FillRule::OddEven);
						}
					}
					if (style->getStrokeEnable())
					{
						const float d = std::sqrt(std::abs(Mshape.determinant()));
						const float w = d * style->getStrokeWidth();
						const int32_t st = raster.defineSolidStyle(style->getStroke() * opacity);
						raster.stroke(st, w, drawing::Raster::StrokeJoin::Round, drawing::Raster::StrokeCap::Butt);
					}
				}

				raster.submit();
			}
			return true;
		},
		[&](Shape* shape)
		{
		}
	);

	const_cast< Document* >(document)->visit(&visitor);
	return true;
}

Ref< drawing::Image > Rasterizer::raster(const Document* document, float scale) const
{
	const int32_t width = (int32_t)(document->getSize().x * scale);
	const int32_t height = (int32_t)(document->getSize().y * scale);
	if (width <= 0 || height <= 0)
		return nullptr;

	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), width, height);
	image->clear(Color4f(1.0f, 1.0f, 1.0f, 0.0f));
	
	if (!raster(document, image))
		return nullptr;

	return image;
}

}
