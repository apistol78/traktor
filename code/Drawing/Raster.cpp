#include <agg_alpha_mask_u8.h>
#include <agg_conv_curve.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_pixfmt_gray.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_compound_aa.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_base.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_scanline_p.h>
#include <agg_scanline_u.h>
#include <agg_span_allocator.h>
#include "Core/Containers/AlignedVector.h"
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
	virtual void setMask(Image* image) = 0;

	virtual void clearStyles() = 0;

	virtual int32_t defineStyle(const Color4f& color) = 0;

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

	virtual void fill(int32_t style0, int32_t style1, Raster::FillRuleType fillRule) = 0;

	virtual void stroke(int32_t style, float width, Raster::StrokeCapType cap) = 0;

	virtual void submit() = 0;
};

// AGG interface for custom styles.
template< typename color_type >
class StyleHandler
{
};

template < >
class StyleHandler< agg::gray8 >
{
public:
	void clearStyles()
	{
		m_colors.resize(0);
	}

	int32_t defineStyle(const Color4f& color)
	{
		m_colors.push_back(agg::gray8(
			agg::int8u(color.getAlpha() * 255.0f)
		));
		return int32_t(m_colors.size() - 1);
	}

	bool is_solid(unsigned style) const
	{
		return true;
	}

    agg::gray8 color(unsigned style) const 
    {
        if (style < m_colors.size())
            return m_colors[style];
		else
			return agg::gray8(0);
    }

    void generate_span(agg::gray8* span, int x, int y, unsigned len, unsigned style)
    {
		T_FATAL_ERROR;
    }

private:
	AlignedVector< agg::gray8 > m_colors;
};

template < >
class StyleHandler< agg::rgba8 >
{
public:
	void clearStyles()
	{
		m_colors.resize(0);
	}

	int32_t defineStyle(const Color4f& color)
	{
		m_colors.push_back(agg::rgba8(
			agg::int8u(color.getRed() * 255.0f),
			agg::int8u(color.getGreen() * 255.0f),
			agg::int8u(color.getBlue() * 255.0f),
			agg::int8u(color.getAlpha() * 255.0f)
		));
		return int32_t(m_colors.size() - 1);
	}

	bool is_solid(unsigned style) const
	{
		return true;
	}

    agg::rgba8 color(unsigned style) const 
    {
        if (style < m_colors.size())
            return m_colors[style];
		else
			return agg::rgba8(0, 0, 0, 0);
    }

    void generate_span(agg::rgba8* span, int x, int y, unsigned len, unsigned style)
    {
		T_FATAL_ERROR;
    }

private:
	AlignedVector< agg::rgba8 > m_colors;
};

template < typename pixfmt_type, typename color_type >
class RasterImpl : public RefCountImpl< IRasterImpl >
{
public:
	RasterImpl(Image* image)
	:	m_rbuffer((agg::int8u*)image->getData(), image->getWidth(), image->getHeight(), image->getWidth() * image->getPixelFormat().getByteSize())
	,	m_pf(m_rbuffer)
	,	m_renderer(m_pf)
	{
	}

	virtual void setMask(Image* image) T_OVERRIDE T_FINAL
	{
		m_mask = image;
	}

	virtual void clearStyles() T_OVERRIDE T_FINAL
	{
		m_styleHandler.clearStyles();
	}

	virtual int32_t defineStyle(const Color4f& color) T_OVERRIDE T_FINAL
	{
		return m_styleHandler.defineStyle(color);
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

	virtual void fill(int32_t style0, int32_t style1, Raster::FillRuleType fillRule) T_OVERRIDE T_FINAL
	{
		agg::conv_curve< agg::path_storage > curve(m_path);
		
		if (fillRule == Raster::FrNonZero)
			m_rasterizer.filling_rule(agg::fill_non_zero);
		else // Raster::FrOddEven
			m_rasterizer.filling_rule(agg::fill_even_odd);

		m_rasterizer.styles(style0, style1);
		m_rasterizer.add_path(curve);
	}

	virtual void stroke(int32_t style, float width, Raster::StrokeCapType cap) T_OVERRIDE T_FINAL
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

		m_rasterizer.filling_rule(agg::fill_non_zero);
		m_rasterizer.styles(-1, style);
		m_rasterizer.add_path(outline);
	}

	virtual void submit() T_OVERRIDE T_FINAL
	{
		agg::span_allocator< color_type > alloc;
		if (!m_mask)
		{
			agg::scanline_u8 sl;
			agg::render_scanlines_compound_layered(m_rasterizer, sl, m_renderer, alloc, m_styleHandler);
		}
		else
		{
			agg::rendering_buffer mrb((agg::int8u*)m_mask->getData(), m_mask->getWidth(), m_mask->getHeight(), m_mask->getWidth() * m_mask->getPixelFormat().getByteSize());
			agg::alpha_mask_gray8 mask(mrb);
			agg::scanline_u8_am< agg::alpha_mask_gray8 > sl(mask);
			agg::render_scanlines_compound_layered(m_rasterizer, sl, m_renderer, alloc, m_styleHandler);
		}
		m_rasterizer.reset();
	}

private:
	StyleHandler< color_type > m_styleHandler;
	Ref< Image > m_mask;
	agg::rendering_buffer m_rbuffer;
	pixfmt_type m_pf;
	agg::renderer_base< pixfmt_type > m_renderer;
	agg::rasterizer_compound_aa<> m_rasterizer;
	agg::path_storage m_path;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.Raster", Raster, Object)

Raster::Raster()
{
}

Raster::Raster(Image* image)
{
	setImage(image);
}

bool Raster::valid() const
{
	return m_impl != 0;
}

bool Raster::setImage(Image* image)
{
	m_impl = 0;

	if (image->getPixelFormat() == PixelFormat::getA8B8G8R8())
		m_impl = new RasterImpl< agg::pixfmt_rgba32, agg::rgba8 >(image);
	else if (image->getPixelFormat() == PixelFormat::getB8G8R8A8())
		m_impl = new RasterImpl< agg::pixfmt_argb32, agg::rgba8 >(image);
	else if (image->getPixelFormat() == PixelFormat::getA8R8G8B8())
		m_impl = new RasterImpl< agg::pixfmt_bgra32, agg::rgba8 >(image);
	else if (image->getPixelFormat() == PixelFormat::getR8G8B8A8())
		m_impl = new RasterImpl< agg::pixfmt_abgr32, agg::rgba8 >(image);
	else if (image->getPixelFormat() == PixelFormat::getA8())
		m_impl = new RasterImpl< agg::pixfmt_gray8, agg::gray8 >(image);

	return valid();
}

void Raster::setMask(Image* image)
{
	m_impl->setMask(image);
}

void Raster::clearStyles()
{
	m_impl->clearStyles();
}

int32_t Raster::defineStyle(const Color4f& color)
{
	return m_impl->defineStyle(color);
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

void Raster::fill(int32_t style0, int32_t style1, FillRuleType fillRule)
{
	m_impl->fill(style0, style1, fillRule);
}

void Raster::stroke(int32_t style, float width, StrokeCapType cap)
{
	m_impl->stroke(style, width, cap);
}

void Raster::submit()
{
	m_impl->submit();
}

	}
}
