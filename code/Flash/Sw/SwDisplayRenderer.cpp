#include "Drawing/Image.h"
#include "Drawing/Raster.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashShape.h"
#include "Flash/Sw/SwDisplayRenderer.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.SwDisplayRenderer", SwDisplayRenderer, IDisplayRenderer)

SwDisplayRenderer::SwDisplayRenderer(drawing::Image* image, bool clearBackground)
:	m_image(image)
,	m_transform(Matrix33::identity())
,	m_clearBackground(clearBackground)
,	m_writeMask(false)
,	m_writeEnable(true)
{
	m_raster = new drawing::Raster(m_image);
}

void SwDisplayRenderer::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

void SwDisplayRenderer::setImage(drawing::Image* image)
{
	T_ASSERT (image->getPixelFormat() == m_image->getPixelFormat());
	m_image = image;
	m_raster = new drawing::Raster(m_image);
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
	if (m_clearBackground)
		m_image->clear(Color4f(
			backgroundColor.red / 255.0f,
			backgroundColor.green / 255.0f,
			backgroundColor.blue / 255.0f,
			0.0f
		));
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
	m_writeMask = true;
	if (increment)
	{
		m_writeEnable = true;
		m_mask.push_back(new drawing::Image(
			drawing::PixelFormat::getA8(),
			m_image->getWidth(),
			m_image->getHeight()
		));
		m_raster->setImage(m_mask.back());
		if (m_mask.size() >= 2)
			m_raster->setMask(m_mask.back() - 1);
		else
			m_raster->setMask(0);
	}
	else
	{
		m_writeEnable = false;
		T_FATAL_ASSERT (!m_mask.empty());
		m_mask.pop_back();
	}
}

void SwDisplayRenderer::endMask()
{
	T_FATAL_ASSERT(m_writeMask);
	m_writeMask = false;
	m_writeEnable = true;
	m_raster->setImage(m_image);
	if (!m_mask.empty())
		m_raster->setMask(m_mask.back());
	else
		m_raster->setMask(0);
}

void SwDisplayRenderer::renderShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform, uint8_t blendMode)
{
	if (!m_writeEnable)
		return;

	const AlignedVector< FlashFillStyle >& fillStyles = shape.getFillStyles();
	AlignedVector< int32_t > fsm;

	// Convert all fill styles used by this shape.
	m_raster->clearStyles();
	if (!m_writeMask)
	{
		for (uint32_t i = 0; i < uint32_t(fillStyles.size()); ++i)
		{
			const FlashFillStyle& style = fillStyles[i];
			const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords = style.getColorRecords();
			if (colorRecords.size() >= 1)
			{
				fsm.push_back(m_raster->defineStyle(Color4f(
					colorRecords[0].color.red / 255.0f,
					colorRecords[0].color.green / 255.0f,
					colorRecords[0].color.blue / 255.0f,
					colorRecords[0].color.alpha / 255.0f
				)));
			}
			else
				fsm.push_back(-1);
		}
	}
	else
	{
		m_raster->defineStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
	}

	int32_t width = m_image->getWidth();
	int32_t height = m_image->getHeight();
	float frameWidth = m_frameBounds.mx.x;
	float frameHeight = m_frameBounds.mx.y;

	Matrix33 rasterTransform = traktor::scale(width / frameWidth, height / frameHeight) * transform * m_transform;

	// Rasterize every path in shape.
	const AlignedVector< Path >& paths = shape.getPaths();
	for (AlignedVector< Path >::const_iterator i = paths.begin(); i != paths.end(); ++i)
	{
		const AlignedVector< Vector2 >& points = i->getPoints();
		const AlignedVector< SubPath >& subPaths = i->getSubPaths();
		for (AlignedVector< SubPath >::const_iterator j = subPaths.begin(); j != subPaths.end(); ++j)
		{
			int32_t fs0 = j->fillStyle0 - 1;
			int32_t fs1 = j->fillStyle1 - 1;

			if (!fs0 && !fs1)
				continue;

			m_raster->clear();

			const AlignedVector< SubPathSegment >& segments = j->segments;
			for (AlignedVector< SubPathSegment >::const_iterator k = segments.begin(); k != segments.end(); ++k)
			{
				m_raster->moveTo(rasterTransform * points[k->pointsOffset]);
				if (k->type == SpgtLinear)
					m_raster->lineTo(rasterTransform * points[k->pointsOffset + 1]);
				else
					m_raster->quadricTo(rasterTransform * points[k->pointsOffset + 1], rasterTransform * points[k->pointsOffset + 2]);
			}

			if (!m_writeMask)
				m_raster->fill(fs0, fs1, drawing::Raster::FrNonZero);
			else
				m_raster->fill(fs0 >= 0 ? 0 : -1, fs1 >= 0 ? 0 : -1, drawing::Raster::FrNonZero);
		}

		m_raster->submit();
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

void SwDisplayRenderer::renderCanvas(const Matrix33& transform, const FlashCanvas& canvas, const SwfCxTransform& cxform)
{
}

void SwDisplayRenderer::end()
{
}

	}
}
