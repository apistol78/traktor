/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Types.h"
#include "Spark/IDisplayRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class IRenderSystem;
class IRenderTargetSet;
class RenderGraph;
class RenderPass;

}

namespace traktor::spark
{

class AccGlyph;
class AccGradientCache;
class AccQuad;
class AccShape;
class AccShapeResources;
class AccShapeVertexPool;
class AccTextureCache;

/*! Accelerated display renderer.
 * \ingroup Spark
 *
 * This display renderer uses the render system
 * in order to accelerate rendering of SWF shapes.
 */
class T_DLLCLASS AccDisplayRenderer : public IDisplayRenderer
{
	T_RTTI_CLASS;

public:
	AccDisplayRenderer();

	virtual ~AccDisplayRenderer();

	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t frameCount,
		bool clearBackground
	);

	void destroy();

	void beginSetup(render::RenderGraph* renderGraph);

	void endSetup();

	void flushCaches();

	void setClearBackground(bool clearBackground);

	// \name IDisplayRenderer
	// \{

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
		float fontSize,
		wchar_t character,
		const Color4f& color,
		const ColorTransform& cxform,
		uint8_t filter,
		const Color4f& filterColor
	) override final;

	virtual void renderQuad(const Matrix33& transform, const Aabb2& bounds, const ColorTransform& cxform) override final;

	virtual void renderCanvas(const Matrix33& transform, const Canvas& canvas, const ColorTransform& cxform, uint8_t blendMode) override final;

	virtual void end() override final;

	// \}

private:
	struct ShapeCache
	{
		Ref< AccShape > shape;
		int32_t unusedCount;
		int32_t tag;
	};

	struct GlyphCache
	{
		Ref< AccShape > shape;
		int32_t index;
	};

	resource::IResourceManager* m_resourceManager;
	render::IRenderSystem* m_renderSystem;
	Ref< render::RenderGraph > m_renderGraph;
	Ref< render::RenderPass > m_renderPassOutput;
	Ref< render::RenderPass > m_renderPassGlyph;
	Ref< AccShapeResources > m_shapeResources;
	Ref< AccShapeVertexPool > m_fillVertexPool;
	Ref< AccShapeVertexPool > m_lineVertexPool;
	Ref< AccGradientCache > m_gradientCache;
	Ref< AccTextureCache > m_textureCache;
	Ref< AccGlyph > m_glyph;
	Ref< AccQuad > m_quad;
	SmallMap< int32_t, ShapeCache > m_shapeCache;
	SmallMap< int32_t, GlyphCache > m_glyphCache;
	render::handle_t m_glyphsTargetSetId;
	int32_t m_nextIndex;
	Vector4 m_frameBounds;			//!< [left, top, right, bottom] in twips.
	Vector4 m_frameTransform;		//!< [offset x, offset y, scale x, scale y] in normalized values.
	Vector4 m_viewSize;				//!< [width, height, 1/width, 1/height] in pixels.
	Aabb2 m_frameBoundsVisible;		//!< Part of frame visible on screen, in twips and in "frame" space.
	Aabb2 m_dirtyRegion;
	bool m_clearBackground;
	bool m_maskWrite;
	bool m_maskIncrement;
	uint8_t m_maskReference;
	uint8_t m_glyphFilter;
	Color4f m_glyphColor;
	Color4f m_glyphFilterColor;
	bool m_firstFrame;

	void renderEnqueuedGlyphs();
};

}
