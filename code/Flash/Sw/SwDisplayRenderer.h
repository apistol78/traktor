#ifndef traktor_flash_SwDisplayRenderer_H
#define traktor_flash_SwDisplayRenderer_H

#include "Flash/IDisplayRenderer.h"

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

/*! \brief Software display renderer.
 * \ingroup Flash
 */
class T_DLLCLASS SwDisplayRenderer : public IDisplayRenderer
{
	T_RTTI_CLASS;

public:
	SwDisplayRenderer();

	void setTransform(const Matrix33& transform);

	void setRasterTarget(void* bits, uint32_t width, uint32_t height, uint32_t pitch);
	
	virtual void begin(
		const FlashDictionary& dictionary,
		const SwfColor& backgroundColor,
		const Aabb2& frameBounds,
		float viewWidth,
		float viewHeight,
		const Vector4& viewOffset
	) T_OVERRIDE T_FINAL;

	virtual void beginMask(bool increment) T_OVERRIDE T_FINAL;

	virtual void endMask() T_OVERRIDE T_FINAL;

	virtual void renderShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform, uint8_t blendMode) T_OVERRIDE T_FINAL;

	virtual void renderMorphShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform) T_OVERRIDE T_FINAL;

	virtual void renderGlyph(const FlashDictionary& dictionary, const Matrix33& transform, const Vector2& fontMaxDimension, const FlashShape& glyphShape, const SwfColor& color, const SwfCxTransform& cxform, uint8_t filter, const SwfColor& filterColor) T_OVERRIDE T_FINAL;

	virtual void renderQuad(const Matrix33& transform, const Aabb2& bounds, const SwfCxTransform& cxform) T_OVERRIDE T_FINAL;

	virtual void renderCanvas(const FlashDictionary& dictionary, const Matrix33& transform, const FlashCanvas& canvas, const SwfCxTransform& cxform) T_OVERRIDE T_FINAL;

	virtual void end() T_OVERRIDE T_FINAL;

private:
	struct Span
	{
		float x;
		float dx;
		uint16_t fillStyle;
	};

	typedef std::vector< Span > spanline_t;

	Matrix33 m_transform;
	Aabb2 m_frameBounds;

	void* m_bits;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_pitch;
	std::vector< spanline_t > m_spanlines;

	void insertSpan(spanline_t& spanline, float x, float dx, uint16_t fillStyle);
};

	}
}

#endif	// traktor_flash_SwDisplayRenderer_H
