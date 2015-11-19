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

class FlashCanvas;
class FlashDictionary;
class FlashSpriteInstance;
class FlashShape;
class FlashMorphShape;

/*! \brief Rendering interface.
 * \ingroup Flash
 */
class T_DLLCLASS IDisplayRenderer : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Begin rendering frame.
	 *
	 * \param dictionary Flash character dictionary.
	 * \param backgroundColor Frame background color.
	 * \param frameBounds Frame bounds.
	 * \param viewWidth View width in pixels.
	 * \param viewHeight View height in pixels.
	 * \param viewOffset View transformation; determined by stage alignment etc.
	 */
	virtual void begin(
		const FlashDictionary& dictionary,
		const SwfColor& backgroundColor,
		const Aabb2& frameBounds,
		float viewWidth,
		float viewHeight,
		const Vector4& viewOffset
	) = 0;

	/*! \brief Begin rendering sprite.
	 */
	virtual void beginSprite(const FlashSpriteInstance& sprite) = 0;

	/*! \brief End rendering sprite.
	 */
	virtual void endSprite(const FlashSpriteInstance& sprite) = 0;

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
	virtual void renderShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform, uint8_t blendMode) = 0;

	/*! \brief Render morph shape.
	 *
	 * \param dictionary Flash character dictionary.
	 * \param transform Shape transform.
	 * \param shape Shape
	 * \param cxform Color transform.
	 */
	virtual void renderMorphShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform) = 0;

	/*! \brief Render glyph.
	 *
	 * \param dictionary Flash character dictionary.
	 * \param transform Shape transform.
	 * \param fontMaxDimension Max font glyph size.
	 * \param glyphShape Shape
	 * \param color Color
	 * \param cxform Color transform.
	 */
	virtual void renderGlyph(const FlashDictionary& dictionary, const Matrix33& transform, const Vector2& fontMaxDimension, const FlashShape& glyphShape, const SwfColor& color, const SwfCxTransform& cxform, uint8_t filter, const SwfColor& filterColor) = 0;

	/*! \brief Render solid quad.
	 *
	 * \param transform Shape transform.
	 * \param bounds Quad bounds.
	 * \param color Color
	 * \param cxform Color transform.
	 */
	virtual void renderQuad(const Matrix33& transform, const Aabb2& bounds, const SwfCxTransform& cxform) = 0;

	/*! \brief Render canvas.
	 *
	 * \param dictionary Flash character dictionary.
	 * \param transform Shape transform.
	 * \param canvas Canvas
	 * \param cxform Color transform.
	 */
	virtual void renderCanvas(const FlashDictionary& dictionary, const Matrix33& transform, const FlashCanvas& canvas, const SwfCxTransform& cxform) = 0;

	/*! \brief End frame. */
	virtual void end() = 0;
};

	}
}

#endif	// traktor_flash_IDisplayRenderer_H
