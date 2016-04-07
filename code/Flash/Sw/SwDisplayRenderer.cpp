#include "Drawing/Image.h"
#include "Drawing/Raster.h"
#include "Flash/FlashBitmapImage.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashShape.h"
#include "Flash/Sw/SwDisplayRenderer.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const static Matrix33 c_textureTS = translate(0.5f, 0.5f) * scale(1.0f / 32768.0f, 1.0f / 32768.0f);

		}

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
		m_mask.back()->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
		m_raster->setImage(m_mask.back());
		if (m_mask.size() >= 2)
			m_raster->setMask(m_mask[m_mask.size() - 2]);
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

	const Color4f cxm(cxform.red[0], cxform.green[0], cxform.blue[0], cxform.alpha[0]);
	const Color4f cxa(cxform.red[1], cxform.green[1], cxform.blue[1], cxform.alpha[1]);
	int32_t width = m_image->getWidth();
	int32_t height = m_image->getHeight();
	float frameWidth = m_frameBounds.mx.x;
	float frameHeight = m_frameBounds.mx.y;

	Matrix33 rasterTransform = traktor::scale(width / frameWidth, height / frameHeight) * transform * m_transform;
	float strokeScale = std::min(width / frameWidth, height / frameHeight);

	const AlignedVector< FlashFillStyle >& fillStyles = shape.getFillStyles();
	const AlignedVector< FlashLineStyle >& lineStyles = shape.getLineStyles();
	int32_t lineStyleBase = 0;

	// Convert all styles used by this shape.
	m_raster->clearStyles();
	if (!m_writeMask)
	{
		for (uint32_t i = 0; i < uint32_t(fillStyles.size()); ++i)
		{
			const FlashFillStyle& style = fillStyles[i];
			const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords = style.getColorRecords();

			const FlashBitmapImage* bitmap = dynamic_type_cast< const FlashBitmapImage* >(dictionary.getBitmap(style.getFillBitmap()));
			if (bitmap)
			{
				const drawing::Image* image = bitmap->getImage();
				T_ASSERT (image);

				m_raster->defineImageStyle(
					style.getFillBitmapMatrix().inverse() * rasterTransform.inverse(),
					image,
					style.getFillBitmapRepeat()
				);
			}
			else
			{
				if (colorRecords.size() == 1)
				{
					Color4f c(
						colorRecords[0].color.red / 255.0f,
						colorRecords[0].color.green / 255.0f,
						colorRecords[0].color.blue / 255.0f,
						colorRecords[0].color.alpha / 255.0f
					);
					m_raster->defineSolidStyle(c * cxm + cxa);
				}
				else if (colorRecords.size() > 1)
				{
					switch (style.getGradientType())
					{
					case FlashFillStyle::GtLinear:
						{
							AlignedVector< std::pair< Color4f, float > > colors;
							for (uint32_t j = 0; j < colorRecords.size(); ++j)
							{
								Color4f c(
									colorRecords[j].color.red / 255.0f,
									colorRecords[j].color.green / 255.0f,
									colorRecords[j].color.blue / 255.0f,
									colorRecords[j].color.alpha / 255.0f
								);
								colors.push_back(std::make_pair(c * cxm + cxa, colorRecords[j].ratio));
							}
							m_raster->defineLinearGradientStyle(
								c_textureTS * style.getGradientMatrix().inverse() * rasterTransform.inverse(),
								colors
							);
						}
						break;

					case FlashFillStyle::GtRadial:
						{
							AlignedVector< std::pair< Color4f, float > > colors;
							for (uint32_t j = 0; j < colorRecords.size(); ++j)
							{
								Color4f c(
									colorRecords[j].color.red / 255.0f,
									colorRecords[j].color.green / 255.0f,
									colorRecords[j].color.blue / 255.0f,
									colorRecords[j].color.alpha / 255.0f
								);
								colors.push_back(std::make_pair(c * cxm + cxa, colorRecords[j].ratio));
							}
							m_raster->defineRadialGradientStyle(
								c_textureTS * style.getGradientMatrix().inverse() * rasterTransform.inverse(),
								colors
							);
						}
						break;

					default:
						m_raster->defineSolidStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
						break;
					}
				}
				else
				{
					m_raster->defineSolidStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
				}
			}
		}
		lineStyleBase = int32_t(fillStyles.size());
		for (uint32_t i = 0; i < uint32_t(lineStyles.size()); ++i)
		{
			const FlashLineStyle& style = lineStyles[i];
			Color4f c(
				style.getLineColor().red / 255.0f,
				style.getLineColor().green / 255.0f,
				style.getLineColor().blue / 255.0f,
				style.getLineColor().alpha / 255.0f
			);
			m_raster->defineSolidStyle(c * cxm + cxa);
		}
	}
	else
	{
		m_raster->defineSolidStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
	}

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
			int32_t ls = j->lineStyle - 1;

			T_ASSERT (fs0 >= 0 || fs1 >= 0 || ls >= 0);

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

			if (fs0 >= 0 || fs1 >= 0)
			{
				if (!m_writeMask)
					m_raster->fill(fs0, fs1, drawing::Raster::FrNonZero);
				else
					m_raster->fill(fs0 >= 0 ? 0 : -1, fs1 >= 0 ? 0 : -1, drawing::Raster::FrNonZero);
			}

			if (ls >= 0)
			{
				if (!m_writeMask)
					m_raster->stroke(lineStyleBase + ls, lineStyles[ls].getLineWidth() * strokeScale, drawing::Raster::ScSquare);
				else
					m_raster->stroke(0, lineStyles[ls].getLineWidth(), drawing::Raster::ScSquare);
			}
		}

		m_raster->submit();
	}
}

void SwDisplayRenderer::renderMorphShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform)
{
}

void SwDisplayRenderer::renderGlyph(const FlashDictionary& dictionary, const Matrix33& transform, const Vector2& fontMaxDimension, const FlashShape& glyphShape, const SwfColor& color, const SwfCxTransform& cxform, uint8_t filter, const SwfColor& filterColor)
{
	if (!m_writeEnable)
		return;

	// Add single style for glyph, using color transform to tint glyph.
	m_raster->clearStyles();
	m_raster->defineSolidStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));

	int32_t width = m_image->getWidth();
	int32_t height = m_image->getHeight();
	float frameWidth = m_frameBounds.mx.x;
	float frameHeight = m_frameBounds.mx.y;

	Matrix33 rasterTransform = traktor::scale(width / frameWidth, height / frameHeight) * transform * m_transform;

	// Rasterize every path in shape.
	const AlignedVector< Path >& paths = glyphShape.getPaths();
	for (AlignedVector< Path >::const_iterator i = paths.begin(); i != paths.end(); ++i)
	{
		const AlignedVector< Vector2 >& points = i->getPoints();
		const AlignedVector< SubPath >& subPaths = i->getSubPaths();
		for (AlignedVector< SubPath >::const_iterator j = subPaths.begin(); j != subPaths.end(); ++j)
		{
			int32_t fs0 = j->fillStyle0 - 1;
			int32_t fs1 = j->fillStyle1 - 1;

			T_ASSERT (fs0 >= 0 || fs1 >= 0);

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

			if (fs0 >= 0 || fs1 >= 0)
				m_raster->fill(fs0 >= 0 ? 0 : -1, fs1 >= 0 ? 0 : -1, drawing::Raster::FrNonZero);
		}

		m_raster->submit();
	}

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
