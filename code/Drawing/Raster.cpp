#include <algorithm>
#include <cmath>
#include "Core/Misc/Align.h"
#include "Drawing/Image.h"
#include "Drawing/Raster.h"

namespace traktor
{
	namespace drawing
	{
		namespace
		{

float frac(float v)
{
	return v - int32_t(v);
}

		}

Raster::Raster(Image* image)
:	m_image(image)
{
	m_spanlines.resize(m_image->getHeight());
}

void Raster::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color4f& color)
{
	float dx = float(x1 - x0);
	float dy = float(y1 - y0);

	drawPixel(x0, y0, color);
	
	if (std::abs(dx) > std::abs(dy) && dx != 0)
	{
		if (dx < 0)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		float k = dy / dx;
		float y = y0 + k;

		for (int32_t x = x0 + 1; x < x1; ++x)
		{
			drawPixel(
				x,
				int32_t(y),
				color,
				1.0f - frac(y)
			);
			drawPixel(
				x,
				int32_t(y) + 1,
				color,
				frac(y)
			);
			y += k;
		}
	}
	else if (dy != 0)
	{
		if (dy < 0)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		float k = dx / dy;
		float x = x0 + k;

		for (int32_t y = y0 + 1; y < y1; ++y)
		{
			drawPixel(
				int32_t(x),
				y,
				color,
				1.0f - frac(x)
			);
			drawPixel(
				int32_t(x) + 1,
				y,
				color,
				frac(x)
			);
			x += k;
		}
	}
}

void Raster::drawPixel(int32_t x, int32_t y, const Color4f& color)
{
	Color4f colorDst;
	m_image->getPixel(x, y, colorDst);
	Scalar alpha = color.getAlpha();
	m_image->setPixel(x, y, color * alpha + colorDst * (Scalar(1.0f) - alpha));
}

void Raster::drawPixel(int32_t x, int32_t y, const Color4f& color, float alpha)
{
	Color4f colorDst;
	m_image->getPixel(x, y, colorDst);
	alpha *= color.getAlpha();
	m_image->setPixel(x, y, color * Scalar(alpha) + colorDst * Scalar(1.0f - alpha));
}

void Raster::drawCircle(int32_t x0, int32_t y0, int32_t radius, const Color4f& color)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	drawPixel(x0, y0 + radius, color);
	drawPixel(x0, y0 - radius, color);
	drawPixel(x0 + radius, y0, color);
	drawPixel(x0 - radius, y0, color);

	while (x < y)
	{
		if (f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x;

		drawPixel(x0 + x, y0 + y, color);
		drawPixel(x0 - x, y0 + y, color);
		drawPixel(x0 + x, y0 - y, color);
		drawPixel(x0 - x, y0 - y, color);
		drawPixel(x0 + y, y0 + x, color);
		drawPixel(x0 - y, y0 + x, color);
		drawPixel(x0 + y, y0 - x, color);
		drawPixel(x0 - y, y0 - x, color);
	}
}

void Raster::drawFilledCircle(int32_t x, int32_t y, int32_t radius, const Color4f& color)
{
	for (int32_t yy = -radius; yy <= radius; ++yy)
	{
		int32_t xx = (int32_t)(std::sqrt(float(radius * radius - yy * yy)) + 0.5f);

		int32_t x1 = x - xx;
		int32_t x2 = x + xx;

		for (int32_t ix = x1; ix <= x2; ++ix)
			drawPixel(ix, y + yy, color);
	}
}

void Raster::drawRectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const Color4f& color)
{
	for (int32_t y = y1; y <= y2; ++y)
	{
		drawPixel(x1, y, color);
		drawPixel(x2, y, color);
	}
	for (int32_t x = x1; x<= x2; ++x)
	{
		drawPixel(x, y1, color);
		drawPixel(x, y2, color);
	}
}

void Raster::drawFilledRectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const Color4f& color)
{
	for (int32_t y = y1; y <= y2; ++y)
	{
		for (int32_t x = x1; x<= x2; ++x)
		{
			drawPixel(x, y, color);
		}
	}
}

void Raster::drawPolygon(const Vector2* points, uint32_t npoints, const Color4f& color)
{
	int32_t mny = m_image->getHeight() - 1;
	int32_t mxy = 0;

	float px1 = points[0].x;
	float py1 = points[0].y;

	for (uint32_t i = 1; i < npoints; ++i)
	{
		float px2 = points[i].x;
		float py2 = points[i].y;

		float x1 = px1, y1 = py1;
		float x2 = px2, y2 = py2;

		if (y1 == y2)
		{
			px1 = px2;
			continue;
		}

		int8_t fillDelta = 1;
		if (y1 > y2)
		{
			std::swap(x1, x2);
			std::swap(y1, y2);
			fillDelta = -1;
		}

		int32_t iy1 = int32_t(y1);
		int32_t iy2 = int32_t(y2);

		if (iy1 >= m_image->getHeight() || iy2 < 0)
		{
			px1 = px2;
			py1 = py2;
			continue;
		}

		float dx = (x2 - x1) / (y2 - y1);

		if (iy1 < 0)
		{
			x1 += -iy1 * dx;
			iy1 = 0;
		}

		if (iy2 > m_image->getHeight())
			iy2 = m_image->getHeight();

		mny = std::min< int32_t >(mny, iy1);
		mxy = std::max< int32_t >(mxy, iy2);

		for (int32_t y = iy1; y < iy2; ++y)
		{
			Span span = { x1, fillDelta };
			insertSpan(m_spanlines[y], span);
			x1 += dx;
		}

		px1 = px2;
		py1 = py2;
	}

	for (int32_t y = mny; y < mxy; ++y)
	{
		spanline_t& spanline = m_spanlines[y];

		int32_t spansize = int32_t(spanline.size());
		if (spansize <= 1)
		{
			spanline.resize(0);
			continue;
		}

		int32_t count = spanline[0].fillDelta;
		int32_t ix1 = int32_t(spanline[0].x);

		for (int32_t i = 1; i < spansize && ix1 < m_image->getWidth(); ++i)
		{
			int32_t ix2 = int32_t(spanline[i].x);

			if (count == 0 || ix2 < 0)
			{
				ix1 = ix2;
				count += spanline[i].fillDelta;
				continue;
			}

			if (ix1 < 0)
				ix1 = 0;

			if (ix2 >= m_image->getWidth())
				ix2 = m_image->getWidth() - 1;

			for (int32_t x = ix1; x <= ix2; ++x)
				m_image->setPixelUnsafe(x, y, color);

			ix1 = ix2 + 1;
			count += spanline[i].fillDelta;
		}

		spanline.resize(0);
	}
}

void Raster::drawPolyLine(const Vector2* points, uint32_t npoints, const Color4f& color)
{
	int32_t px = int32_t(points[0].x);
	int32_t py = int32_t(points[0].y);
	for (uint32_t i = 1; i < npoints; ++i)
	{
		const Vector2& n = points[i];

		int32_t nx = int32_t(n.x);
		int32_t ny = int32_t(n.y);

		drawLine(px, py, nx, ny, color);

		px = nx;
		py = ny;
	}
}

void Raster::insertSpan(spanline_t& spanline, const Span& span) const
{
	for (size_t i = 0; i < spanline.size(); ++i)
	{
		if (span.x < spanline[i].x)
		{
			spanline.insert(spanline.begin() + i, span);
			return;
		}
	}
	spanline.push_back(span);
}

	}
}
