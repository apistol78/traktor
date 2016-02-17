#pragma optimize( "", off )

#include <algorithm>
#include <limits>
#include "Core/Math/Bezier2nd.h"
#include "Drawing/Image.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashShape.h"
#include "Flash/Sw/SwDisplayRenderer.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

struct Edge
{
	SubPathSegmentType type;
	std::vector< Vector2 > points;

	Edge(SubPathSegmentType type_ = SpgtUndefined)
	:	type(type_)
	{}
};

float scale(float v, float fromDim, float toDim)
{
	return (v * toDim) / fromDim;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.SwDisplayRenderer", SwDisplayRenderer, IDisplayRenderer)

SwDisplayRenderer::SwDisplayRenderer(drawing::Image* image)
:	m_image(image)
,	m_transform(Matrix33::identity())
{
	m_spanlines.resize(m_image->getHeight());
}

void SwDisplayRenderer::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

bool SwDisplayRenderer::wantDirtyRegion() const
{
	return false;
}

void SwDisplayRenderer::begin(
	const FlashDictionary& dictionary,
	const SwfColor& backgroundColor,
	const Aabb2& frameBounds,
	const Vector4& frameTransform,
	float viewWidth,
	float viewHeight,
	const Aabb2& dirtyRegion
)
{
	/*
	uint32_t color = castColor(backgroundColor);
	m_image->clear();	
	*/
	m_frameBounds = frameBounds;
}

void SwDisplayRenderer::beginSprite(const FlashSpriteInstance& sprite, const Matrix33& transform)
{
}

void SwDisplayRenderer::endSprite(const FlashSpriteInstance& sprite, const Matrix33& transform)
{
}

void SwDisplayRenderer::beginMask(bool increment)
{
}

void SwDisplayRenderer::endMask()
{
}

void SwDisplayRenderer::renderShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform, uint8_t blendMode)
{
	const AlignedVector< FlashFillStyle >& fillStyles = shape.getFillStyles();
	Matrix33 rasterTransform = transform * m_transform;
	Edge edge;
	Color4f color;
	Color4f target;

	int32_t width = m_image->getWidth();
	int32_t height = m_image->getHeight();
	float frameWidth = m_frameBounds.mx.x;
	float frameHeight = m_frameBounds.mx.y;
	float screenScale = ((width / frameWidth) + (height / frameHeight)) / 2.0f;

	const AlignedVector< Path >& paths = shape.getPaths();
	for (AlignedVector< Path >::const_iterator i = paths.begin(); i != paths.end(); ++i)
	{
		const AlignedVector< Vector2 >& points = i->getPoints();

		// Create spans for each path.
		const AlignedVector< SubPath >& subPaths = i->getSubPaths();
		for (AlignedVector< SubPath >::const_iterator j = subPaths.begin(); j != subPaths.end(); ++j)
		{
			const AlignedVector< SubPathSegment >& segments = j->segments;

			// Transform segments into linear edges.
			static AlignedVector< Edge > edges;
			edges.resize(0);

			for (AlignedVector< SubPathSegment >::const_iterator k = segments.begin(); k != segments.end(); ++k)
			{
				if (k->type == SpgtLinear)
				{
					edge.type = SpgtLinear;
					edge.points.push_back(rasterTransform * points[k->pointsOffset]);
					edge.points.push_back(rasterTransform * points[k->pointsOffset + 1]);
					edges.push_back(edge);
				}
				else
				{
					Vector2 cp[] =
					{
						rasterTransform * points[k->pointsOffset],
						rasterTransform * points[k->pointsOffset + 1],
						rasterTransform * points[k->pointsOffset + 2]
					};

					float area = abs(((cp[1].x - cp[0].x) * (cp[2].y - cp[0].y) - (cp[2].x - cp[0].x) * (cp[1].y - cp[0].y)) / 2.0f);
					float screenArea = area * screenScale;

					int32_t steps = int32_t(sqrtf(screenArea) / 4.0f);
					steps = min(steps, 8);
					steps = max(steps, 1);

					Bezier2nd b(cp[0], cp[1], cp[2]);
					Vector2 p1 = cp[0];
					for (int32_t t = 1; t <= steps; ++t)
					{
						Vector2 p2 = b.evaluate(float(t) / steps);
						edge.type = SpgtLinear;
						edge.points.push_back(p1);
						edge.points.push_back(p2);
						edges.push_back(edge);
						p1 = p2;
					}
				}
			}

			// Generate spans from edges.
			for (AlignedVector< Edge >::const_iterator k = edges.begin(); k != edges.end(); ++k)
			{
				const Vector2& p1 = k->points[0];
				const Vector2& p2 = k->points[1];

				float x1 = p1.x;
				float y1 = p1.y;

				float x2 = p2.x;
				float y2 = p2.y;

				if (y1 == y2)
					continue;

				if (y1 > y2)
				{
					std::swap(x1, x2);
					std::swap(y1, y2);
				}

				int32_t iy1 = (int32_t)scale(y1, frameHeight, float(height));
				int32_t iy2 = (int32_t)scale(y2, frameHeight, float(height));

				float dx = (x2 - x1) / (y2 - y1);

				uint16_t fillStyle;
				if ((transform.e11 >= 0.0f) ^ !(transform.e22 >= 0.0f))
					fillStyle = p1.y >= p2.y ? j->fillStyle0 : j->fillStyle1;
				else
					fillStyle = p1.y >= p2.y ? j->fillStyle1 : j->fillStyle0;

				for (int32_t y = iy1 - 1; y <= iy2 + 1; ++y)
				{
					float fy = scale(float(y), float(height), frameHeight);
					if (fy < y1 || fy >= y2 || y < 0 || y >= height)
						continue;

					float x = x1 + (fy - y1) * dx;
					insertSpan(m_spanlines[y], x, dx, fillStyle);
				}
			}
		}

		// Draw spans.
		for (int32_t y = 0; y < height; ++y)
		{
			spanline_t& spanline = m_spanlines[y];

			if (spanline.empty())
				continue;

			spanline_t::iterator j = spanline.begin();
			float x1 = scale(j->x, frameWidth, float(width));

			while (++j != spanline.end())
			{
				float x2 = scale(j->x, frameWidth, float(width));
				uint16_t f = j->fillStyle;
				if (f)
				{
					float cx1 = x1;
					if (cx1 < 0.0f)
						cx1 = 0.0f;
					if (cx1 >= width)
						cx1 = width - 1.0f;

					float cx2 = x2;
					if (cx2 < 0.0f)
						cx2 = 0.0f;
					if (cx2 >= width)
						cx2 = width - 1.0f;

					if (cx1 < cx2)
					{
						const FlashFillStyle& style = fillStyles[f - 1];
						const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords = style.getColorRecords();
						
						int32_t icx1 = int32_t(cx1);
						int32_t icx2 = int32_t(cx2);

						if (colorRecords.size() == 1)
						{
							color = Color4f(
								colorRecords[0].color.red / 255.0f,
								colorRecords[0].color.green / 255.0f,
								colorRecords[0].color.blue / 255.0f,
								colorRecords[0].color.alpha / 255.0f
							);

							color = color * Color4f(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]) + Color4f(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]);

							Scalar alpha = color.getAlpha();
							Scalar alphaInv = Scalar(1.0f) - alpha;

							for (int32_t x = icx1; x <= icx2; ++x)
							{
								m_image->getPixelUnsafe(x, y, target);
								m_image->setPixelUnsafe(x, y, (color * alpha + target * alphaInv).rgb1());
							}
						}
						else
						{
							T_ASSERT (colorRecords.size() > 1);

							if (style.getGradientType() == FlashFillStyle::GtLinear)
							{
							}
							else if (style.getGradientType() == FlashFillStyle::GtRadial)
							{
							}
						}
					}
				}
				x1 = x2;
			}

			spanline.resize(0);
		}
	}
}

void SwDisplayRenderer::renderMorphShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform)
{
}

void SwDisplayRenderer::renderGlyph(const FlashDictionary& dictionary, const Matrix33& transform, const Vector2& fontMaxDimension, const FlashShape& glyphShape, const SwfColor& color, const SwfCxTransform& cxform, uint8_t filter, const SwfColor& filterColor)
{
}

void SwDisplayRenderer::renderQuad(const Matrix33& transform, const Aabb2& bounds, const SwfCxTransform& cxform)
{
}

void SwDisplayRenderer::renderCanvas(const FlashDictionary& dictionary, const Matrix33& transform, const FlashCanvas& canvas, const SwfCxTransform& cxform)
{
}

void SwDisplayRenderer::end()
{
}

void SwDisplayRenderer::insertSpan(spanline_t& spanline, float x, float dx, uint16_t fillStyle)
{
	Span span = { x, dx, fillStyle };
	for (spanline_t::iterator i = spanline.begin(); i != spanline.end(); ++i)
	{
		if (span.x < i->x || (span.x == i->x && span.dx < i->dx))
		{
			spanline.insert(i, span);
			return;
		}
	}
	spanline.push_back(span);
}

	}
}
