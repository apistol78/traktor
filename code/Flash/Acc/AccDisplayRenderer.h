/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AccDisplayRenderer_H
#define traktor_flash_AccDisplayRenderer_H

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Flash/IDisplayRenderer.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;
class RenderContext;
class RenderTargetSet;

	}

	namespace flash
	{

class AccGlyph;
class AccGradientCache;
class AccQuad;
class AccShape;
class AccShapeRenderer;
class AccShapeResources;
class AccShapeVertexPool;
class AccTextureCache;

/*! \brief Accelerated display renderer.
 * \ingroup Flash
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
		uint32_t renderContextSize,
		bool clearBackground,
		bool shapeCache,
		bool clipToDirtyRegion,
		float stereoscopicOffset
	);

	void destroy();

	void build(uint32_t frame);

	void build(render::RenderContext* renderContext, uint32_t frame);

	void render(render::IRenderView* renderView, uint32_t frame, render::EyeType eye, const Vector2& offset, float scale);

	void flush();

	void flushCaches();

	// \name IDisplayRenderer
	// \{

	virtual bool wantDirtyRegion() const T_OVERRIDE T_FINAL;
	
	virtual void begin(
		const Dictionary& dictionary,
		const Color4f& backgroundColor,
		const Aabb2& frameBounds,
		const Vector4& frameTransform,
		float viewWidth,
		float viewHeight,
		const Aabb2& dirtyRegion
	) T_OVERRIDE T_FINAL;

	virtual void beginSprite(const SpriteInstance& sprite, const Matrix33& transform) T_OVERRIDE T_FINAL;

	virtual void endSprite(const SpriteInstance& sprite, const Matrix33& transform) T_OVERRIDE T_FINAL;

	virtual void beginEdit(const EditInstance& edit, const Matrix33& transform) T_OVERRIDE T_FINAL;

	virtual void endEdit(const EditInstance& edit, const Matrix33& transform) T_OVERRIDE T_FINAL;

	virtual void beginMask(bool increment) T_OVERRIDE T_FINAL;

	virtual void endMask() T_OVERRIDE T_FINAL;

	virtual void renderShape(const Dictionary& dictionary, const Matrix33& transform, const Shape& shape, const ColorTransform& cxform, uint8_t blendMode) T_OVERRIDE T_FINAL;

	virtual void renderMorphShape(const Dictionary& dictionary, const Matrix33& transform, const MorphShape& shape, const ColorTransform& cxform) T_OVERRIDE T_FINAL;

	virtual void renderGlyph(
		const Dictionary& dictionary,
		const Matrix33& transform,
		const Font* font,
		const Shape* glyph,
		float fontSize,
		wchar_t character,
		const Color4f& color,
		const ColorTransform& cxform,
		uint8_t filter,
		const Color4f& filterColor
	) T_OVERRIDE T_FINAL;

	virtual void renderQuad(const Matrix33& transform, const Aabb2& bounds, const ColorTransform& cxform) T_OVERRIDE T_FINAL;

	virtual void renderCanvas(const Matrix33& transform, const Canvas& canvas, const ColorTransform& cxform, uint8_t blendMode) T_OVERRIDE T_FINAL;

	virtual void end() T_OVERRIDE T_FINAL;

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
	RefArray< render::RenderContext > m_renderContexts;
	Ref< render::RenderContext > m_renderContext;
	Ref< render::RenderTargetSet > m_renderTargetGlyphs;
	Ref< render::RenderTargetSet > m_frameTarget;
	Ref< AccShapeResources > m_shapeResources;
	Ref< AccShapeVertexPool > m_vertexPool;
	Ref< AccShapeRenderer > m_shapeRenderer;
	Ref< AccGradientCache > m_gradientCache;
	Ref< AccTextureCache > m_textureCache;
	Ref< AccGlyph > m_glyph;
	Ref< AccQuad > m_quad;
	SmallMap< int32_t, ShapeCache > m_shapeCache;
	SmallMap< int32_t, GlyphCache > m_glyphCache;
	int32_t m_nextIndex;
	Vector4 m_frameBounds;			//!< [left, top, right, bottom] in twips.
	Vector4 m_frameTransform;		//!< [offset x, offset y, scale x, scale y] in normalized values.
	Vector4 m_viewSize;				//!< [width, height, 1/width, 1/height] in pixels.
	Aabb2 m_frameBoundsVisible;		//!< Part of frame visible on screen, in twips and in "frame" space.
	Aabb2 m_dirtyRegion;
	bool m_clearBackground;
	bool m_clipToDirtyRegion;
	bool m_maskWrite;
	bool m_maskIncrement;
	uint8_t m_maskReference;
	uint8_t m_glyphFilter;
	Color4f m_glyphColor;
	Color4f m_glyphFilterColor;

	void renderEnqueuedGlyphs();
};

	}
}

#endif	// traktor_flash_AccDisplayRenderer_H
