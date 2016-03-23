#include <agg_conv_curve.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_base.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_scanline_p.h>
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Drawing/Image.h"
#include "Drawing/Raster.h"

namespace traktor
{
	namespace drawing
	{

class IRasterImpl : public IRefCount
{
public:
	virtual void clear() = 0;

	virtual void moveTo(float x, float y) = 0;

	virtual void lineTo(float x, float y) = 0;

	virtual void quadricTo(float x1, float y1, float x, float y) = 0;

	virtual void quadricTo(float x, float y) = 0;

	virtual void cubicTo(float x1, float y1, float x2, float y2, float x, float y) = 0;

	virtual void cubicTo(float x2, float y2, float x, float y) = 0;

	virtual void close() = 0;

	virtual void rect(float x, float y, float width, float height, float radius) = 0;

	virtual void circle(float x, float y, float radius) = 0;

	virtual void fill(const Color4f& color) = 0;

	virtual void stroke(const Color4f& color, float width, Raster::StrokeCapType cap) = 0;
};

template < typename pixfmt_type >
class RasterImpl : public RefCountImpl< IRasterImpl >
{
public:
	RasterImpl(Image* image)
	:	m_rbuffer((agg::int8u*)image->getData(), image->getWidth(), image->getHeight(), image->getWidth() * image->getPixelFormat().getByteSize())
	,	m_pf(m_rbuffer)
	,	m_renderer(m_pf)
	{
	}

	virtual void clear() T_OVERRIDE T_FINAL
	{
		m_path.remove_all();
	}

	virtual void moveTo(float x, float y) T_OVERRIDE T_FINAL
	{
		m_path.move_to(x, y);
	}

	virtual void lineTo(float x, float y) T_OVERRIDE T_FINAL
	{
		m_path.line_to(x, y);
	}

	virtual void quadricTo(float x1, float y1, float x, float y) T_OVERRIDE T_FINAL
	{
		m_path.curve3(x1, y1, x, y);
	}

	virtual void quadricTo(float x, float y) T_OVERRIDE T_FINAL
	{
		m_path.curve3(x, y);
	}

	virtual void cubicTo(float x1, float y1, float x2, float y2, float x, float y) T_OVERRIDE T_FINAL
	{
		m_path.curve4(x1, y1, x2, y2, x, y);
	}

	virtual void cubicTo(float x2, float y2, float x, float y) T_OVERRIDE T_FINAL
	{
		m_path.curve4(x2, y2, x, y);
	}

	virtual void close() T_OVERRIDE T_FINAL
	{
		m_path.close_polygon();
	}

	virtual void rect(float x, float y, float width, float height, float radius) T_OVERRIDE T_FINAL
	{
	}

	virtual void circle(float x, float y, float radius) T_OVERRIDE T_FINAL
	{
	}

	virtual void fill(const Color4f& color) T_OVERRIDE T_FINAL
	{
		agg::conv_curve< agg::path_storage > curve(m_path);

		agg::rasterizer_scanline_aa<> rasterizer;
		rasterizer.reset();
		rasterizer.add_path(curve);

		agg::renderer_scanline_aa_solid< agg::renderer_base< pixfmt_type > > renderer(m_renderer);
		renderer.color(agg::rgba(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha()));

		agg::scanline_p8 scanline;
		agg::render_scanlines(rasterizer, scanline, renderer);
	}

	virtual void stroke(const Color4f& color, float width, Raster::StrokeCapType cap) T_OVERRIDE T_FINAL
	{
		agg::conv_stroke< agg::path_storage > outline(m_path);
		outline.width(width);

		switch (cap)
		{
		case Raster::ScButt:
			outline.line_cap(agg::butt_cap);
			break;

		case Raster::ScSquare:
			outline.line_cap(agg::square_cap);
			break;

		case Raster::ScRound:
			outline.line_cap(agg::round_cap);
			break;

		default:
			break;
		}

		agg::rasterizer_scanline_aa<> rasterizer;
		rasterizer.reset();
		rasterizer.add_path(outline);

		agg::renderer_scanline_aa_solid< agg::renderer_base< pixfmt_type > > renderer(m_renderer);
		renderer.color(agg::rgba(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha()));

		agg::scanline_p8 scanline;
		agg::render_scanlines(rasterizer, scanline, renderer);
	}

private:
	agg::rendering_buffer m_rbuffer;
	pixfmt_type m_pf;
	agg::renderer_base< pixfmt_type > m_renderer;
	agg::path_storage m_path;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.Raster", Raster, Object)

Raster::Raster(Image* image)
{
	if (image->getPixelFormat() == PixelFormat::getA8B8G8R8())
		m_impl = new RasterImpl< agg::pixfmt_rgba32 >(image);
	else if (image->getPixelFormat() == PixelFormat::getB8G8R8A8())
		m_impl = new RasterImpl< agg::pixfmt_argb32 >(image);
	else if (image->getPixelFormat() == PixelFormat::getA8R8G8B8())
		m_impl = new RasterImpl< agg::pixfmt_bgra32 >(image);
	else if (image->getPixelFormat() == PixelFormat::getR8G8B8A8())
		m_impl = new RasterImpl< agg::pixfmt_abgr32 >(image);
	else
		log::error << L"Unsupported pixel format in Raster." << Endl;
}

bool Raster::valid() const
{
	return m_impl != 0;
}

void Raster::clear()
{
	m_impl->clear();
}

void Raster::moveTo(float x, float y)
{
	m_impl->moveTo(x, y);
}

void Raster::lineTo(float x, float y)
{
	m_impl->lineTo(x, y);
}

void Raster::quadricTo(float x1, float y1, float x, float y)
{
	m_impl->quadricTo(x1, y1, x, y);
}

void Raster::quadricTo(float x, float y)
{
	m_impl->quadricTo(x, y);
}

void Raster::cubicTo(float x1, float y1, float x2, float y2, float x, float y)
{
	m_impl->cubicTo(x1, y1, x2, y2, x, y);
}

void Raster::cubicTo(float x2, float y2, float x, float y)
{
	m_impl->cubicTo(x2, y2, x, y);
}

void Raster::close()
{
	m_impl->close();
}

void Raster::rect(float x, float y, float width, float height, float radius)
{
	m_impl->rect(x, y, width, height, radius);
}

void Raster::circle(float x, float y, float radius)
{
	m_impl->circle(x, y, radius);
}

void Raster::fill(const Color4f& color)
{
	m_impl->fill(color);
}

void Raster::stroke(const Color4f& color, float width, StrokeCapType cap)
{
	m_impl->stroke(color, width, cap);
}

	}
}
