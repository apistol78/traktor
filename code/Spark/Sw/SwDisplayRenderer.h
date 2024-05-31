/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Spark/IDisplayRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;
class Raster;

}

namespace traktor::spark
{

/*! Software display renderer.
 * \ingroup Spark
 */
class T_DLLCLASS SwDisplayRenderer : public IDisplayRenderer
{
	T_RTTI_CLASS;

public:
	SwDisplayRenderer(drawing::Image* image, bool clearBackground);

	void setTransform(const Matrix33& transform);

	void setImage(drawing::Image* image);

	virtual bool wantDirtyRegion() const override final;

	virtual void begin(
		const Dictionary& dictionary,
		const Color4f& backgroundColor,
		const Aabb2& frameBounds,
		const Vector4& frameTransform,
		float viewWidth,
		float viewHeight,
		const Aabb2& dirtyRegion
	) override final;

	virtual void beginSprite(const SpriteInstance& sprite, const Matrix33& transform) override final;

	virtual void endSprite(const SpriteInstance& sprite, const Matrix33& transform) override final;

	virtual void beginEdit(const EditInstance& edit, const Matrix33& transform) override final;

	virtual void endEdit(const EditInstance& edit, const Matrix33& transform) override final;

	virtual void beginMask(bool increment) override final;

	virtual void endMask() override final;

	virtual void renderShape(const Dictionary& dictionary, const Matrix33& transform, const Aabb2& clipBounds, const Shape& shape, const ColorTransform& cxform, uint8_t blendMode) override final;

	virtual void renderMorphShape(const Dictionary& dictionary, const Matrix33& transform, const Aabb2& clipBounds, const MorphShape& shape, const ColorTransform& cxform) override final;

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
	) override final;

	virtual void renderQuad(const Matrix33& transform, const Aabb2& bounds, const ColorTransform& cxform) override final;

	virtual void renderCanvas(const Matrix33& transform, const Canvas& canvas, const ColorTransform& cxform, uint8_t blendMode) override final;

	virtual void end() override final;

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
