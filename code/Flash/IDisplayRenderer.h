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
class FlashMovie;
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
	/*! \brief Preload movie resources.
	 *
	 * \param movie Flash movie.
	 */
	virtual void preload(const FlashMovie& movie) = 0;
	
	/*! \brief Begin rendering frame.
	 *
	 * \param movie Flash movie.
	 * \param backgroundColor Frame background color.
	 */
	virtual void begin(const FlashMovie& movie, const SwfColor& backgroundColor) = 0;

	/*! \brief Begin rendering mask.
	 *
	 * \param increment Increment mask.
	 */
	virtual void beginMask(bool increment) = 0;

	/*! \brief End rendering mask. */
	virtual void endMask() = 0;

	/*! \brief Render shape.
	 *
	 * \param movie Flash movie.
	 * \param transform Shape transform.
	 * \param shape Shape
	 * \param cxform Color transform.
	 */
	virtual void renderShape(const FlashMovie& movie, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform) = 0;

	/*! \brief Render morph shape.
	 *
	 * \param movie Flash movie.
	 * \param transform Shape transform.
	 * \param shape Shape
	 * \param cxform Color transform.
	 */
	virtual void renderMorphShape(const FlashMovie& movie, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform) = 0;

	/*! \brief Render glyph.
	 *
	 * \param movie Flash movie.
	 * \param transform Shape transform.
	 * \param glyphShape Shape
	 * \param color Color
	 * \param cxform Color transform.
	 */
	virtual void renderGlyph(const FlashMovie& movie, const Matrix33& transform, const FlashShape& glyphShape, const SwfColor& color, const SwfCxTransform& cxform) = 0;

	/*! \brief Render canvas.
	 *
	 * \param movie Flash movie.
	 * \param transform Shape transform.
	 * \param canvas Canvas
	 * \param cxform Color transform.
	 */
	virtual void renderCanvas(const FlashMovie& movie, const Matrix33& transform, const FlashCanvas& canvas, const SwfCxTransform& cxform) = 0;

	/*! \brief End frame. */
	virtual void end() = 0;
};

	}
}

#endif	// traktor_flash_IDisplayRenderer_H
