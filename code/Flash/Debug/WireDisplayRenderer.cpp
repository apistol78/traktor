/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashShape.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/TextLayout.h"
#include "Flash/Action/ActionObject.h"
#include "Flash/Debug/WireDisplayRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{
		
Vector4 transformIntoView(const Aabb2& frameBounds, const Vector4& frameTransform, const Matrix33& transform, const Vector2& pt)
{
	Vector2 pt1 = transform * pt;

	Vector2 fb_xy = frameBounds.mn;
	Vector2 fb_wz = frameBounds.mx;

	Vector2 ft_xy(frameTransform.x(), frameTransform.y());
	Vector2 ft_zw(frameTransform.z(), frameTransform.w());

	Vector2 p0 = (pt1 - fb_xy) / (fb_wz - fb_xy);
	Vector2 p1 = p0 * ft_zw + ft_xy;
	Vector2 p2 = p1 * Vector2(2.0f, -2.0f) + Vector2(-1.0f, 1.0f);

	return Vector4(p2.x, p2.y, 0.0f, 1.0f);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.WireDisplayRenderer", WireDisplayRenderer, IDisplayRenderer)

WireDisplayRenderer::WireDisplayRenderer(IDisplayRenderer* displayRenderer)
:	m_displayRenderer(displayRenderer)
{
}

bool WireDisplayRenderer::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t frameCount
)
{
	m_primitiveRenderer = new render::PrimitiveRenderer();
	m_primitiveRenderer->create(resourceManager, renderSystem, frameCount);
	return true;
}

void WireDisplayRenderer::begin(uint32_t frame)
{
	m_primitiveRenderer->begin(frame, orthoLh(2.0f, 2.0f, 0.0f, 1.0f));
	m_primitiveRenderer->pushWorld(Matrix44::identity());
	m_primitiveRenderer->pushView(Matrix44::identity());
}

void WireDisplayRenderer::end(uint32_t frame)
{
	m_primitiveRenderer->popView();
	m_primitiveRenderer->popWorld();
	m_primitiveRenderer->end(frame);
}

void WireDisplayRenderer::render(render::IRenderView* renderView, uint32_t frame)
{
	m_primitiveRenderer->render(renderView, frame);
}

bool WireDisplayRenderer::wantDirtyRegion() const
{
	return m_displayRenderer->wantDirtyRegion();
}

void WireDisplayRenderer::begin(
	const FlashDictionary& dictionary,
	const Color4f& backgroundColor,
	const Aabb2& frameBounds,
	const Vector4& frameTransform,
	float viewWidth,
	float viewHeight,
	const Aabb2& dirtyRegion
)
{
	m_displayRenderer->begin(dictionary, backgroundColor, frameBounds, frameTransform, viewWidth, viewHeight, dirtyRegion);

	m_frameBounds = frameBounds;
	m_frameTransform = frameTransform;

	m_wireEnable.push(false);
}

void WireDisplayRenderer::beginSprite(const FlashSpriteInstance& sprite, const Matrix33& transform)
{
	m_displayRenderer->beginSprite(sprite, transform);

	bool parentWireEnable = m_wireEnable.top();

	ActionValue wireOutline;	
	if (sprite.getAsObject()->getMember("__renderWireOutline", wireOutline) && wireOutline.getBoolean())
		m_wireEnable.push(true);
	else
		m_wireEnable.push(false);

	if (parentWireEnable || m_wireEnable.top())
	{
		const Aabb2& bounds = sprite.getLocalBounds();

		const Vector2& mn = bounds.mn;
		const Vector2& mx = bounds.mx;

		if (m_wireEnable.top())
		{
			// Pivot
			m_primitiveRenderer->drawSolidPoint(
				transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(0.0f, 0.0f)),
				2.0f,
				Color4ub(255, 0, 0, 200)
			);

			// Line from pivot to top-left corner of bounds.
			m_primitiveRenderer->drawLine(
				transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(0.0f, 0.0f)),
				transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mn.y)),
				Color4ub(255, 0, 0, 200)
			);
		}

		// Bounds.
		m_primitiveRenderer->drawSolidQuad(
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mn.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mx.x, mn.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mx.x, mx.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mx.y)),
			m_wireEnable.top() ? Color4ub(255, 0, 0, 20) : Color4ub(255, 255, 0, 20)
		);

		m_primitiveRenderer->drawWireQuad(
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mn.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mx.x, mn.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mx.x, mx.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mx.y)),
			m_wireEnable.top() ? Color4ub(255, 0, 0, 255) : Color4ub(255, 255, 0, 255)
		);
	}
}

void WireDisplayRenderer::endSprite(const FlashSpriteInstance& sprite, const Matrix33& transform)
{
	m_displayRenderer->endSprite(sprite, transform);
	m_wireEnable.pop();
}

void WireDisplayRenderer::beginEdit(const FlashEditInstance& edit, const Matrix33& transform)
{
	m_displayRenderer->beginEdit(edit, transform);

	bool parentWireEnable = m_wireEnable.top();

	ActionValue wireOutline;	
	if (edit.getAsObject()->getMember("__renderWireOutline", wireOutline) && wireOutline.getBoolean())
		m_wireEnable.push(true);
	else
		m_wireEnable.push(false);

	if (parentWireEnable || m_wireEnable.top())
	{
		const Aabb2& bounds = edit.getTextBounds();

		const Vector2& mn = bounds.mn;
		const Vector2& mx = bounds.mx;

		// Bounds.
		m_primitiveRenderer->drawSolidQuad(
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mn.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mx.x, mn.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mx.x, mx.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mx.y)),
			m_wireEnable.top() ? Color4ub(255, 0, 255, 20) : Color4ub(0, 255, 255, 20)
		);

		m_primitiveRenderer->drawWireQuad(
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mn.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mx.x, mn.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mx.x, mx.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mx.y)),
			m_wireEnable.top() ? Color4ub(255, 0, 255, 255) : Color4ub(0, 255, 255, 255)
		);

		if (m_wireEnable.top())
		{
			const TextLayout* layout = edit.getTextLayout();
			T_ASSERT (layout);

			const AlignedVector< TextLayout::Line >& lines = layout->getLines();
			const AlignedVector< TextLayout::Attribute >& attribs = layout->getAttributes();

			float textOffsetX = 0.0f;
			float textOffsetY = -(layout->getFontHeight() + layout->getLeading()) * edit.getScroll();

			for (AlignedVector< TextLayout::Line >::const_iterator i = lines.begin(); i != lines.end(); ++i)
			{
				for (AlignedVector< TextLayout::Word >::const_iterator j = i->words.begin(); j != i->words.end(); ++j)
				{
					const TextLayout::Attribute& attrib = attribs[j->a];
					const AlignedVector< TextLayout::Character >& chars = j->chars;

					float coordScale = attrib.font->getCoordinateType() == FlashFont::CtTwips ? 1.0f / 1000.0f : 1.0f / (20.0f * 1000.0f);
					float fontScale = coordScale * layout->getFontHeight();

					for (uint32_t k = 0; k < chars.size(); ++k)
					{
						if (chars[k].ch != 0)
						{
							uint16_t glyphIndex = attrib.font->lookupIndex(chars[k].ch);

							const FlashShape* glyphShape = attrib.font->getShape(glyphIndex);
							if (!glyphShape)
								continue;

							Matrix33 glyphTransform = transform * translate(textOffsetX + i->x + chars[k].x, textOffsetY + i->y) * scale(fontScale, fontScale);

							const Aabb2& shapeBounds = glyphShape->getShapeBounds();

							const Vector2& mn = shapeBounds.mn;
							const Vector2& mx = shapeBounds.mx;

							m_primitiveRenderer->drawWireQuad(
								transformIntoView(m_frameBounds, m_frameTransform, glyphTransform, Vector2(mn.x, mn.y)),
								transformIntoView(m_frameBounds, m_frameTransform, glyphTransform, Vector2(mx.x, mn.y)),
								transformIntoView(m_frameBounds, m_frameTransform, glyphTransform, Vector2(mx.x, mx.y)),
								transformIntoView(m_frameBounds, m_frameTransform, glyphTransform, Vector2(mn.x, mx.y)),
								Color4ub(0, 0, 255, 128)
							);
						}
					}
				}
			}
		}
	}
}

void WireDisplayRenderer::endEdit(const FlashEditInstance& edit, const Matrix33& transform)
{
	m_displayRenderer->endEdit(edit, transform);
	m_wireEnable.pop();
}

void WireDisplayRenderer::beginMask(bool increment)
{
	m_displayRenderer->beginMask(increment);
}

void WireDisplayRenderer::endMask()
{
	m_displayRenderer->endMask();
}

void WireDisplayRenderer::renderShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashShape& shape, const ColorTransform& cxform, uint8_t blendMode)
{
	m_displayRenderer->renderShape(dictionary, transform, shape, cxform, blendMode);

	if (m_wireEnable.top())
	{
		const Aabb2& bounds = shape.getShapeBounds();

		const Vector2& mn = bounds.mn;
		const Vector2& mx = bounds.mx;

		m_primitiveRenderer->drawSolidQuad(
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mn.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mx.x, mn.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mx.x, mx.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mx.y)),
			Color4ub(0, 255, 0, 20)
		);

		m_primitiveRenderer->drawWireQuad(
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mn.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mx.x, mn.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mx.x, mx.y)),
			transformIntoView(m_frameBounds, m_frameTransform, transform, Vector2(mn.x, mx.y)),
			Color4ub(0, 255, 0, 255)
		);
	}
}

void WireDisplayRenderer::renderMorphShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashMorphShape& shape, const ColorTransform& cxform)
{
	m_displayRenderer->renderMorphShape(dictionary, transform, shape, cxform);
}

void WireDisplayRenderer::renderGlyph(
	const FlashDictionary& dictionary,
	const Matrix33& transform,
	const FlashFont* font,
	const FlashShape* glyph,
	float fontHeight,
	wchar_t character,
	const Color4f& color,
	const ColorTransform& cxform,
	uint8_t filter,
	const Color4f& filterColor
)
{
	m_displayRenderer->renderGlyph(dictionary, transform, font, glyph, fontHeight, character, color, cxform, filter, filterColor);
}

void WireDisplayRenderer::renderQuad(const Matrix33& transform, const Aabb2& bounds, const ColorTransform& cxform)
{
	m_displayRenderer->renderQuad(transform, bounds, cxform);
}

void WireDisplayRenderer::renderCanvas(const Matrix33& transform, const FlashCanvas& canvas, const ColorTransform& cxform, uint8_t blendMode)
{
	m_displayRenderer->renderCanvas(transform, canvas, cxform, blendMode);
}

void WireDisplayRenderer::end()
{
	m_displayRenderer->end();
}

	}
}
