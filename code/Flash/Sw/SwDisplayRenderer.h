/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_SwDisplayRenderer_H
#define traktor_flash_SwDisplayRenderer_H

#include "Core/RefArray.h"
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
	namespace drawing
	{

class Image;
class Raster;

	}

	namespace flash
	{

/*! \brief Software display renderer.
 * \ingroup Flash
 */
class T_DLLCLASS SwDisplayRenderer : public IDisplayRenderer
{
	T_RTTI_CLASS;

public:
	SwDisplayRenderer(drawing::Image* image, bool clearBackground);

	void setTransform(const Matrix33& transform);

	void setImage(drawing::Image* image);

	virtual bool wantDirtyRegion() const T_OVERRIDE T_FINAL;
	
	virtual void begin(
		const FlashDictionary& dictionary,
		const Color4f& backgroundColor,
		const Aabb2& frameBounds,
		const Vector4& frameTransform,
		float viewWidth,
		float viewHeight,
		const Aabb2& dirtyRegion
	) T_OVERRIDE T_FINAL;

	virtual void beginSprite(const FlashSpriteInstance& sprite, const Matrix33& transform) T_OVERRIDE T_FINAL;

	virtual void endSprite(const FlashSpriteInstance& sprite, const Matrix33& transform) T_OVERRIDE T_FINAL;

	virtual void beginEdit(const FlashEditInstance& edit, const Matrix33& transform) T_OVERRIDE T_FINAL;

	virtual void endEdit(const FlashEditInstance& edit, const Matrix33& transform) T_OVERRIDE T_FINAL;

	virtual void beginMask(bool increment) T_OVERRIDE T_FINAL;

	virtual void endMask() T_OVERRIDE T_FINAL;

	virtual void renderShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashShape& shape, const ColorTransform& cxform, uint8_t blendMode) T_OVERRIDE T_FINAL;

	virtual void renderMorphShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashMorphShape& shape, const ColorTransform& cxform) T_OVERRIDE T_FINAL;

	virtual void renderGlyph(
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
	) T_OVERRIDE T_FINAL;

	virtual void renderQuad(const Matrix33& transform, const Aabb2& bounds, const ColorTransform& cxform) T_OVERRIDE T_FINAL;

	virtual void renderCanvas(const Matrix33& transform, const FlashCanvas& canvas, const ColorTransform& cxform, uint8_t blendMode) T_OVERRIDE T_FINAL;

	virtual void end() T_OVERRIDE T_FINAL;

private:
	Ref< drawing::Image > m_image;
	RefArray< drawing::Image > m_mask;
	Ref< drawing::Raster > m_raster;
	Matrix33 m_transform;
	Aabb2 m_frameBounds;
	Vector4 m_frameTransform;
	bool m_clearBackground;
	bool m_writeMask;
	bool m_writeEnable;
};

	}
}

#endif	// traktor_flash_SwDisplayRenderer_H
