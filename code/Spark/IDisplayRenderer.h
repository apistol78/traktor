/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Matrix33.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class ColorTransform;
class Canvas;
class Dictionary;
class EditInstance;
class Font;
class MorphShape;
class Shape;
class SpriteInstance;

/*! Rendering interface.
 * \ingroup Spark
 */
class T_DLLCLASS IDisplayRenderer : public Object
{
	T_RTTI_CLASS;

public:
	/*! Return true if renderer require proper dirty region.
	 */
	virtual bool wantDirtyRegion() const = 0;

	/*! Begin rendering frame.
	 *
	 * \param dictionary Flash character dictionary.
	 * \param backgroundColor Frame background color.
	 * \param frameBounds Frame bounds.
	 * \param frameTransform Frame transformation; determined by stage alignment etc.
	 * \param viewWidth Output view width in pixels.
	 * \param viewHeight Output view height in pixels.
	 * \param dirtyRegion Region of stage which needs to be redrawn.
	 */
	virtual void begin(
		const Dictionary& dictionary,
		const Color4f& backgroundColor,
		const Aabb2& frameBounds,
		const Vector4& frameTransform,
		float viewWidth,
		float viewHeight,
		const Aabb2& dirtyRegion
	) = 0;

	/*! Begin rendering sprite.
	 */
	virtual void beginSprite(const SpriteInstance& sprite, const Matrix33& transform) = 0;

	/*! End rendering sprite.
	 */
	virtual void endSprite(const SpriteInstance& sprite, const Matrix33& transform) = 0;

	/*! Begin rendering edit field. */
	virtual void beginEdit(const EditInstance& edit, const Matrix33& transform) = 0;

	/*! End rendering edit field. */
	virtual void endEdit(const EditInstance& edit, const Matrix33& transform) = 0;

	/*! Begin rendering mask.
	 *
	 * \param increment Increment mask.
	 */
	virtual void beginMask(bool increment) = 0;

	/*! End rendering mask. */
	virtual void endMask() = 0;

	/*! Render shape.
	 *
	 * \param dictionary Flash character dictionary.
	 * \param transform Shape transform.
	 * \param shape Shape
	 * \param cxform Color transform.
	 * \param blendMode Blend mode.
	 */
	virtual void renderShape(const Dictionary& dictionary, const Matrix33& transform, const Aabb2& clipBounds, const Shape& shape, const ColorTransform& cxform, uint8_t blendMode) = 0;

	/*! Render morph shape.
	 *
	 * \param dictionary Flash character dictionary.
	 * \param transform Shape transform.
	 * \param shape Shape
	 * \param cxform Color transform.
	 */
	virtual void renderMorphShape(const Dictionary& dictionary, const Matrix33& transform, const Aabb2& clipBounds, const MorphShape& shape, const ColorTransform& cxform) = 0;

	/*! Render glyph.
	 *
	 * \param dictionary Flash character dictionary.
	 * \param transform Shape transform.
	 * \param font Font description.
	 * \param glyph Glyph shape if available, not available if system font.
	 * \param fontHeight Height of font.
	 * \param character Character, not required if shape is specified.
	 * \param color Color
	 * \param cxform Color transform.
	 * \param filter Text filter.
	 * \param filterColor Text filter color.
	 */
	virtual void renderGlyph(
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
	) = 0;

	/*! Render solid quad.
	 *
	 * \param transform Shape transform.
	 * \param bounds Quad bounds.
	 * \param color Color
	 * \param cxform Color transform.
	 */
	virtual void renderQuad(const Matrix33& transform, const Aabb2& bounds, const ColorTransform& cxform) = 0;

	/*! Render canvas.
	 *
	 * \param transform Shape transform.
	 * \param canvas Canvas
	 * \param cxform Color transform.
	 * \param blendMode Blend mode.
	 */
	virtual void renderCanvas(const Matrix33& transform, const Canvas& canvas, const ColorTransform& cxform, uint8_t blendMode) = 0;

	/*! End frame. */
	virtual void end() = 0;
};

	}
}

