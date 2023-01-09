/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Spark/EditInstance.h"
#include "Spark/Font.h"
#include "Spark/Shape.h"
#include "Spark/SpriteInstance.h"
#include "Spark/TextLayout.h"
#include "Spark/Debug/WireDisplayRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spark
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.WireDisplayRenderer", WireDisplayRenderer, IDisplayRenderer)

WireDisplayRenderer::WireDisplayRenderer()
{
}

bool WireDisplayRenderer::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t frameCount
)
{
	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(resourceManager, renderSystem, frameCount))
	{
		safeDestroy(m_primitiveRenderer);
		return false;
	}
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
	return false;
}

void WireDisplayRenderer::begin(
	const Dictionary& dictionary,
	const Color4f& backgroundColor,
	const Aabb2& frameBounds,
	const Vector4& frameTransform,
	float viewWidth,
	float viewHeight,
	const Aabb2& dirtyRegion
)
{
	m_frameBounds = frameBounds;
	m_frameTransform = frameTransform;

	m_wireEnable.push(false);
}

void WireDisplayRenderer::beginSprite(const SpriteInstance& sprite, const Matrix33& transform)
{
	const bool parentWireEnable = m_wireEnable.top();
	m_wireEnable.push(sprite.getWireOutline());

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

void WireDisplayRenderer::endSprite(const SpriteInstance& sprite, const Matrix33& transform)
{
	m_wireEnable.pop();
}

void WireDisplayRenderer::beginEdit(const EditInstance& edit, const Matrix33& transform)
{
	bool parentWireEnable = m_wireEnable.top();
	m_wireEnable.push(edit.getWireOutline());

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
			T_ASSERT(layout);

			const auto& attribs = layout->getAttributes();
			const float textOffsetX = 0.0f;
			const float textOffsetY = -(layout->getFontHeight() + layout->getLeading()) * edit.getScroll();

			for (const auto& line : layout->getLines())
			{
				for (const auto& word : line.words)
				{
					const TextLayout::Attribute& attrib = attribs[word.a];
					const AlignedVector< TextLayout::Character >& chars = word.chars;

					const float coordScale = attrib.font->getCoordinateType() == Font::CtTwips ? 1.0f / 1000.0f : 1.0f / (20.0f * 1000.0f);
					const float fontScale = coordScale * layout->getFontHeight();

					for (uint32_t k = 0; k < chars.size(); ++k)
					{
						if (chars[k].ch != 0)
						{
							const uint16_t glyphIndex = attrib.font->lookupIndex(chars[k].ch);
							const Shape* glyphShape = attrib.font->getShape(glyphIndex);
							if (!glyphShape)
								continue;

							const Matrix33 glyphTransform = transform * translate(textOffsetX + line.offset + line.x + chars[k].x, textOffsetY + line.y) * scale(fontScale, fontScale);

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

void WireDisplayRenderer::endEdit(const EditInstance& edit, const Matrix33& transform)
{
	m_wireEnable.pop();
}

void WireDisplayRenderer::beginMask(bool increment)
{
}

void WireDisplayRenderer::endMask()
{
}

void WireDisplayRenderer::renderShape(const Dictionary& dictionary, const Matrix33& transform, const Aabb2& clipBounds, const Shape& shape, const ColorTransform& cxform, uint8_t blendMode)
{
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

void WireDisplayRenderer::renderMorphShape(const Dictionary& dictionary, const Matrix33& transform, const Aabb2& clipBounds, const MorphShape& shape, const ColorTransform& cxform)
{
}

void WireDisplayRenderer::renderGlyph(
	const Dictionary& dictionary,
	const Matrix33& transform,
	const Aabb2& clipBounds,
	const Font* font,
	const Shape* glyph,
	float fontHeight,
	wchar_t character,
	const Color4f& color,
	const ColorTransform& cxform,
	uint8_t filter,
	const Color4f& filterColor
)
{
}

void WireDisplayRenderer::renderQuad(const Matrix33& transform, const Aabb2& bounds, const ColorTransform& cxform)
{
}

void WireDisplayRenderer::renderCanvas(const Matrix33& transform, const Canvas& canvas, const ColorTransform& cxform, uint8_t blendMode)
{
}

void WireDisplayRenderer::end()
{
}

	}
}
