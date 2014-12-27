#include <algorithm>
#include <limits>
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

uint32_t castColor(const SwfColor& color)
{
	return 
		(uint32_t(color.red) << 16) |
		(uint32_t(color.green) << 8) |
		(uint32_t(color.blue)) |
		(uint32_t(color.alpha) << 24);
}

uint32_t lerpColor(uint32_t c1, uint32_t c2, float blend)
{
	int r = int(((c1 >> 16) & 255) * (1.0f - blend) + ((c2 >> 16) & 255) * blend);
	int g = int(((c1 >> 8) & 255) * (1.0f - blend) + ((c2 >> 8) & 255) * blend);
	int b = int((c1 & 255) * (1.0f - blend) + (c2 & 255) * blend);
	return (r << 16) | (g << 8) | b;
}

Vector2 evalQuadratic(
	double t,
	const Vector2& cp0,
	const Vector2& cp1,
	const Vector2& cp2
)
{
	double it = 1.0 - t;
	return float(it * it) * cp0 + float(2.0 * it * t) * cp1 + float(t * t) * cp2;
}

float scale(float v, float fromDim, float toDim)
{
	return (v * toDim) / fromDim;
}

bool culled(const Vector2* pts, int npts, float width, float height)
{
	Vector2 mn( std::numeric_limits< float >::max(),  std::numeric_limits< float >::max());
	Vector2 mx(-std::numeric_limits< float >::max(), -std::numeric_limits< float >::max());

	for (int i = 0; i < npts; ++i)
	{
		const Vector2& pt = pts[i];
		mn.x = min(mn.x, pt.x);
		mn.y = min(mn.y, pt.y);
		mx.x = max(mx.x, pt.x);
		mx.y = max(mx.y, pt.y);
	}

	if (mx.x < 0.0f || mx.y < 0.0f || mn.x >= width || mn.y >= height)
		return true;

	return false;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.SwDisplayRenderer", SwDisplayRenderer, IDisplayRenderer)

SwDisplayRenderer::SwDisplayRenderer()
:	m_transform(Matrix33::identity())
,	m_bits(0)
,	m_width(0)
,	m_height(0)
,	m_pitch(0)
{
}

void SwDisplayRenderer::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

void SwDisplayRenderer::setRasterTarget(void* bits, uint32_t width, uint32_t height, uint32_t pitch)
{
	m_bits = bits;
	m_width = width;
	m_height = height;
	m_pitch = pitch;
	m_spanlines.resize(m_height);
}

void SwDisplayRenderer::begin(
	const FlashDictionary& dictionary,
	const SwfColor& backgroundColor,
	const Aabb2& frameBounds,
	float viewWidth,
	float viewHeight,
	const Vector4& viewOffset
)
{
	uint32_t color = castColor(backgroundColor);
	
	uint32_t* bits = static_cast< uint32_t* >(m_bits);
	if (!bits)
		return;

	for (uint32_t y = 0; y < m_height; ++y)
	{
		for (uint32_t x = 0; x < m_width; ++x)
		{
			bits[x] = color;
		}
		bits += m_pitch >> 2;
	}

	m_frameBounds = frameBounds;
}

void SwDisplayRenderer::beginMask(bool increment)
{
}

void SwDisplayRenderer::endMask()
{
}

void SwDisplayRenderer::renderShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform)
{
	Matrix33 rasterTransform = transform * m_transform;

	const AlignedVector< FlashFillStyle >& fillStyles = shape.getFillStyles();

	float frameWidth = m_frameBounds.mx.x;
	float frameHeight = m_frameBounds.mx.y;
	float screenScale = ((m_width / frameWidth) + (m_height / frameHeight)) / 2.0f;

	const std::list< Path >& paths = shape.getPaths();
	for (std::list< Path >::const_iterator i = paths.begin(); i != paths.end(); ++i)
	{
		const std::vector< Vector2i >& points = i->getPoints();

		// Create spans for each path.
		const std::list< SubPath >& subPaths = i->getSubPaths();
		for (std::list< SubPath >::const_iterator j = subPaths.begin(); j != subPaths.end(); ++j)
		{
			const std::vector< SubPathSegment >& segments = j->segments;

			// Transform segments into linear edges.
			static std::vector< Edge > edges;
			edges.resize(0);

			for (std::vector< SubPathSegment >::const_iterator k = segments.begin(); k != segments.end(); ++k)
			{
				if (k->type == SpgtLinear)
				{
					Edge edge;
					edge.type = SpgtLinear;
					edge.points.push_back(rasterTransform * points[k->pointsOffset].toVector2());
					edge.points.push_back(rasterTransform * points[k->pointsOffset + 1].toVector2());
					//if (!culled(&edge.points[0], 2, frameWidth, frameHeight))
						edges.push_back(edge);
				}
				else
				{
					Vector2 cp[] =
					{
						rasterTransform * points[k->pointsOffset].toVector2(),
						rasterTransform * points[k->pointsOffset + 1].toVector2(),
						rasterTransform * points[k->pointsOffset + 2].toVector2()
					};

					//if (!culled(cp, 3, frameWidth, frameHeight))
					{
						float area = abs(((cp[1].x - cp[0].x) * (cp[2].y - cp[0].y) - (cp[2].x - cp[0].x) * (cp[1].y - cp[0].y)) / 2.0f);
						float screenArea = area * screenScale;

						int steps = int(sqrtf(screenArea) / 4.0f);
						steps = min(steps, 8);
						steps = max(steps, 1);

						Vector2 p1 = evalQuadratic(0.0f, cp[0], cp[1], cp[2]);
						for (int t = 1; t <= steps; ++t)
						{
							Vector2 p2 = evalQuadratic(float(t) / steps, cp[0], cp[1], cp[2]);

							Edge edge;
							edge.type = SpgtLinear;
							edge.points.push_back(p1);
							edge.points.push_back(p2);
							edges.push_back(edge);

							p1 = p2;
						}
					}
				}
			}

			// Generate spans from edges.
			for (std::vector< Edge >::const_iterator k = edges.begin(); k != edges.end(); ++k)
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

				int iy1 = (int)scale(y1, frameHeight, float(m_height));
				int iy2 = (int)scale(y2, frameHeight, float(m_height));

				float dx = (x2 - x1) / (y2 - y1);

				uint16_t fillStyle;
				if ((transform.e11 >= 0.0f) ^ !(transform.e22 >= 0.0f))
					fillStyle = p1.y >= p2.y ? j->fillStyle0 : j->fillStyle1;
				else
					fillStyle = p1.y >= p2.y ? j->fillStyle1 : j->fillStyle0;

				for (int y = iy1 - 1; y <= iy2 + 1; ++y)
				{
					float fy = scale(float(y), float(m_height), frameHeight);
					if (fy < y1 || fy >= y2 || y < 0 || y >= int(m_height))
						continue;

					float x = x1 + (fy - y1) * dx;
					insertSpan(m_spanlines[y], x, dx, fillStyle);
				}
			}
		}

		// Draw spans.
		uint32_t* bits = static_cast< uint32_t* >(m_bits);
		for (int y = 0; y < int(m_height); ++y)
		{
			spanline_t& spanline = m_spanlines[y];

			if (spanline.empty())
			{
				bits += m_pitch >> 2;
				continue;
			}

			spanline_t::iterator j = spanline.begin();
			float x1 = scale(j->x, frameWidth, float(m_width));

			while (++j != spanline.end())
			{
				float x2 = scale(j->x, frameWidth, float(m_width));
				uint16_t f = j->fillStyle;
				if (f)
				{
					float cx1 = x1;
					if (cx1 < 0.0f)
						cx1 = 0.0f;
					if (cx1 >= m_width)
						cx1 = m_width - 1.0f;

					float cx2 = x2;
					if (cx2 < 0.0f)
						cx2 = 0.0f;
					if (cx2 >= m_width)
						cx2 = m_width - 1.0f;

					if (cx1 < cx2)
					{
						const FlashFillStyle& style = fillStyles[f - 1];

						const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords = style.getColorRecords();

						int r = int(colorRecords[0].color.red   * cxform.red[0]   + cxform.red[1]   * 255.0f) & 255;
						int g = int(colorRecords[0].color.green * cxform.green[0] + cxform.green[1] * 255.0f) & 255;
						int b = int(colorRecords[0].color.blue  * cxform.blue[0]  + cxform.blue[1]  * 255.0f) & 255;
						int a = int(colorRecords[0].color.alpha * cxform.alpha[0] + cxform.alpha[1] * 255.0f) & 255;

						if (a == 255)
						{
							uint32_t color = (r << 16) | (g << 8) | b;

							int icx1 = int(cx1);
							int icx2 = int(cx2);

							float fcx1 = cx1 - icx1;
							bits[icx1] = lerpColor(bits[icx1], color, 1.0f - fcx1);
							
							for (int x = icx1 + 1; x <= icx2 - 1; ++x)
								bits[x] = color;

							float fcx2 = cx2 - icx2;
							bits[icx2] = lerpColor(bits[icx2], color, fcx2);
						}
						else if (a != 0)
						{
							int icx1 = int(cx1);
							int icx2 = int(cx2);

							for (int x = icx1; x <= icx2; ++x)
							{
								uint32_t dest = bits[x];
								
								int dr = (dest >> 16) & 255;
								int dg = (dest >> 8) & 255;
								int db = (dest) & 255;

								int fr = dr + (((r - dr) * a) >> 8);
								int fg = dg + (((g - dg) * a) >> 8);
								int fb = db + (((b - db) * a) >> 8);

								bits[x] = (fr << 16) | (fg << 8) | fb;
							}
						}
					}
				}
				x1 = x2;
			}

			spanline.resize(0);
			bits += m_pitch >> 2;
		}
	}
}

void SwDisplayRenderer::renderMorphShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform)
{
}

void SwDisplayRenderer::renderGlyph(const FlashDictionary& dictionary, const Matrix33& transform, const Vector2& fontMaxDimension, const FlashShape& glyphShape, const SwfColor& color, const SwfCxTransform& cxform, uint8_t filter, const SwfColor& filterColor)
{
	Matrix33 rasterTransform = transform * m_transform;

	float frameWidth = m_frameBounds.mx.x;
	float frameHeight = m_frameBounds.mx.y;
	float screenScale = ((m_width / frameWidth) + (m_height / frameHeight)) / 2.0f;

	const std::list< Path >& paths = glyphShape.getPaths();
	for (std::list< Path >::const_iterator i = paths.begin(); i != paths.end(); ++i)
	{
		const std::vector< Vector2i >& points = i->getPoints();

		// Create spans for each path.
		const std::list< SubPath >& subPaths = i->getSubPaths();
		for (std::list< SubPath >::const_iterator j = subPaths.begin(); j != subPaths.end(); ++j)
		{
			const std::vector< SubPathSegment >& segments = j->segments;

			// Transform segments into linear edges.
			static std::vector< Edge > edges;
			edges.resize(0);

			for (std::vector< SubPathSegment >::const_iterator k = segments.begin(); k != segments.end(); ++k)
			{
				if (k->type == SpgtLinear)
				{
					Edge edge;
					edge.type = SpgtLinear;
					edge.points.push_back(rasterTransform * points[k->pointsOffset].toVector2());
					edge.points.push_back(rasterTransform * points[k->pointsOffset + 1].toVector2());
					edges.push_back(edge);
				}
				else
				{
					Vector2 cp[] =
					{
						rasterTransform * points[k->pointsOffset].toVector2(),
						rasterTransform * points[k->pointsOffset + 1].toVector2(),
						rasterTransform * points[k->pointsOffset + 2].toVector2()
					};

					{
						float area = abs(((cp[1].x - cp[0].x) * (cp[2].y - cp[0].y) - (cp[2].x - cp[0].x) * (cp[1].y - cp[0].y)) / 2.0f);
						float screenArea = area * screenScale;

						int steps = int(sqrtf(screenArea) / 4.0f);
						steps = min(steps, 8);
						steps = max(steps, 1);

						Vector2 p1 = evalQuadratic(0.0f, cp[0], cp[1], cp[2]);
						for (int t = 1; t <= steps; ++t)
						{
							Vector2 p2 = evalQuadratic(float(t) / steps, cp[0], cp[1], cp[2]);

							Edge edge;
							edge.type = SpgtLinear;
							edge.points.push_back(p1);
							edge.points.push_back(p2);
							edges.push_back(edge);

							p1 = p2;
						}
					}
				}
			}

			// Generate spans from edges.
			for (std::vector< Edge >::const_iterator k = edges.begin(); k != edges.end(); ++k)
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

				int iy1 = (int)scale(y1, frameHeight, float(m_height));
				int iy2 = (int)scale(y2, frameHeight, float(m_height));

				float dx = (x2 - x1) / (y2 - y1);

				for (int y = iy1 - 1; y <= iy2 + 1; ++y)
				{
					float fy = scale(float(y), float(m_height), frameHeight);
					if (fy < y1 || fy >= y2 || y < 0 || y >= int(m_height))
						continue;

					float x = x1 + (fy - y1) * dx;
					insertSpan(m_spanlines[y], x, dx, 0);
				}
			}
		}

		// Draw spans.
		uint32_t* bits = static_cast< uint32_t* >(m_bits);
		for (int y = 0; y < int(m_height); ++y)
		{
			spanline_t& spanline = m_spanlines[y];

			if (spanline.empty())
			{
				bits += m_pitch >> 2;
				continue;
			}

			spanline_t::iterator j = spanline.begin();
			float x1 = scale(j->x, frameWidth, float(m_width));

			while (++j != spanline.end())
			{
				float x2 = scale(j->x, frameWidth, float(m_width));

				float cx1 = x1;
				if (cx1 < 0.0f)
					cx1 = 0.0f;
				if (cx1 >= m_width)
					cx1 = m_width - 1.0f;

				float cx2 = x2;
				if (cx2 < 0.0f)
					cx2 = 0.0f;
				if (cx2 >= m_width)
					cx2 = m_width - 1.0f;

				if (cx1 < cx2)
				{
					int r = color.red;
					int g = color.green;
					int b = color.blue;
					int a = color.alpha;

					if (a == 255)
					{
						uint32_t color = (r << 16) | (g << 8) | b;

						int icx1 = int(cx1);
						int icx2 = int(cx2);

						float fcx1 = cx1 - icx1;
						bits[icx1] = lerpColor(bits[icx1], color, 1.0f - fcx1);

						for (int x = icx1 + 1; x <= icx2 - 1; ++x)
							bits[x] = color;

						float fcx2 = cx2 - icx2;
						bits[icx2] = lerpColor(bits[icx2], color, fcx2);
					}
					else if (a != 0)
					{
						int icx1 = int(cx1);
						int icx2 = int(cx2);

						for (int x = icx1; x <= icx2; ++x)
						{
							uint32_t dest = bits[x];

							int dr = (dest >> 16) & 255;
							int dg = (dest >> 8) & 255;
							int db = (dest) & 255;

							int fr = dr + (((r - dr) * a) >> 8);
							int fg = dg + (((g - dg) * a) >> 8);
							int fb = db + (((b - db) * a) >> 8);

							bits[x] = (fr << 16) | (fg << 8) | fb;
						}
					}
				}
				x1 = x2;
			}

			spanline.resize(0);
			bits += m_pitch >> 2;
		}
	}
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
