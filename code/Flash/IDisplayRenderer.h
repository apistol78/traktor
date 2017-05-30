/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_IDisplayRenderer_H
#define traktor_flash_IDisplayRenderer_H

#include "Core/Object.h"
#include "Core/Math/Matrix33.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ColorTransform;
class Canvas;
class Dictionary;
class EditInstance;
class Font;
class MorphShape;
class Shape;
class SpriteInstance;

/*! \brief Rendering interface.
 * \ingroup Flash
 */
class T_DLLCLASS IDisplayRenderer : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Return true if renderer require proper dirty region.
	 */
	virtual bool wantDirtyRegion() const = 0;

	/*! \brief Begin rendering frame.
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

	/*! \brief Begin rendering sprite.
	 */
	virtual void beginSprite(const SpriteInstance& sprite, const Matrix33& transform) = 0;

	/*! \brief End rendering sprite.
	 */
	virtual void endSprite(const SpriteInstance& sprite, const Matrix33& transform) = 0;

	/*! \brief Begin rendering edit field. */
	virtual void beginEdit(const EditInstance& edit, const Matrix33& transform) = 0;

	/*! \brief End rendering edit field. */
	virtual void endEdit(const EditInstance& edit, const Matrix33& transform) = 0;

	/*! \brief Begin rendering mask.
	 *
	 * \param increment Increment mask.
	 */
	virtual void beginMask(bool increment) = 0;

	/*! \brief End rendering mask. */
	virtual void endMask() = 0;

	/*! \brief Render shape.
	 *
	 * \param dictionary Flash character dictionary.
	 * \param transform Shape transform.
	 * \param shape Shape
	 * \param cxform Color transform.
	 * \param blendMode Blend mode.
	 */
	virtual void renderShape(const Dictionary& dictionary, const Matrix33& transform, const Shape& shape, const ColorTransform& cxform, uint8_t blendMode) = 0;

	/*! \brief Render morph shape.
	 *
	 * \param dictionary Flash character dictionary.
	 * \param transform Shape transform.
	 * \param shape Shape
	 * \param cxform Color transform.
	 */
	virtual void renderMorphShape(const Dictionary& dictionary, const Matrix33& transform, const MorphShape& shape, const ColorTransform& cxform) = 0;

	/*! \brief Render glyph.
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
		const Font* font,
		const Shape* glyph,
		float fontHeight,
		wchar_t character,
		const Color4f& color,
		const ColorTransform& cxform,
		uint8_t filter,
		const Color4f& filterColor
	) = 0;

	/*! \brief Render solid quad.
	 *
	 * \param transform Shape transform.
	 * \param bounds Quad bounds.
	 * \param color Color
	 * \param cxform Color transform.
	 */
	virtual void renderQuad(const Matrix33& transform, const Aabb2& bounds, const ColorTransform& cxform) = 0;

	/*! \brief Render canvas.
	 *
	 * \param transform Shape transform.
	 * \param canvas Canvas
	 * \param cxform Color transform.
	 * \param blendMode Blend mode.
	 */
	virtual void renderCanvas(const Matrix33& transform, const Canvas& canvas, const ColorTransform& cxform, uint8_t blendMode) = 0;

	/*! \brief End frame. */
	virtual void end() = 0;
};

	}
}

#endif	// traktor_flash_IDisplayRenderer_H
