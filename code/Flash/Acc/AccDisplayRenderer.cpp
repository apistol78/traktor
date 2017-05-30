/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/Bitmap.h"
#include "Flash/Canvas.h"
#include "Flash/Dictionary.h"
#include "Flash/EditInstance.h"
#include "Flash/Font.h"
#include "Flash/Movie.h"
#include "Flash/Sprite.h"
#include "Flash/SpriteInstance.h"
#include "Flash/Frame.h"
#include "Flash/Shape.h"
#include "Flash/Acc/AccDisplayRenderer.h"
#include "Flash/Acc/AccGradientCache.h"
#include "Flash/Acc/AccGlyph.h"
#include "Flash/Acc/AccTextureCache.h"
#include "Flash/Acc/AccShape.h"
#include "Flash/Acc/AccShapeRenderer.h"
#include "Flash/Acc/AccShapeResources.h"
#include "Flash/Acc/AccShapeVertexPool.h"
#include "Flash/Acc/AccQuad.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/Context/RenderContext.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const uint32_t c_maxCacheSize = 32;
const uint32_t c_maxUnusedCount = 10;

#if defined(__IOS__) || defined(__ANDROID__) || defined(__PS3__)
const uint32_t c_cacheGlyphSize = 64;
#else
const uint32_t c_cacheGlyphSize = 128;
#endif
const uint32_t c_cacheGlyphMargin = 0;
const uint32_t c_cacheGlyphCountX = 16;
const uint32_t c_cacheGlyphCountY = 8;
const uint32_t c_cacheGlyphCount = c_cacheGlyphCountX * c_cacheGlyphCountY;
const uint32_t c_cacheGlyphDimX = c_cacheGlyphSize * c_cacheGlyphCountX;
const uint32_t c_cacheGlyphDimY = c_cacheGlyphSize * c_cacheGlyphCountY;

const ColorTransform c_cxfZero(Color4f(0.0f, 0.0f, 0.0f, 0.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));
const ColorTransform c_cxfWhite(Color4f(0.0f, 0.0f, 0.0f, 0.0f), Color4f(1.0f, 1.0f, 1.0f, 1.0f));
const ColorTransform c_cxfIdentity(Color4f(1.0f, 1.0f, 1.0f, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));
const ColorTransform c_cxfYellow(Color4f(0.0f, 0.0f, 0.0f, 1.0f), Color4f(1.0f, 1.0f, 0.0f, 0.0f));
const ColorTransform c_cxfDebug(Color4f(0.0f, 0.0f, 0.0f, 0.0f), Color4f(1.0f, 0.0f, 0.0f, 0.2f));

bool rectangleVisible(const Aabb2& frame, const Aabb2& bounds)
{
	if (!frame.empty() && !bounds.empty())
		return !frame.overlapped(bounds).empty();
	else
		return false;
}

bool colorsEqual(const Color4f& a, const Color4f& b)
{
	return Vector4(a - b).absolute().max() <= 2 / 255.0f;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccDisplayRenderer", AccDisplayRenderer, IDisplayRenderer)

AccDisplayRenderer::AccDisplayRenderer()
:	m_vertexPool(0)
,	m_nextIndex(0)
,	m_frameBounds(0.0f, 0.0f, 0.0f, 0.0f)
,	m_frameTransform(0.0f, 0.0f, 1.0f, 1.0f)
,	m_viewSize(0.0f, 0.0f, 0.0f, 0.0f)
,	m_clearBackground(false)
,	m_maskWrite(false)
,	m_maskIncrement(false)
,	m_maskReference(0)
,	m_glyphFilter(0)
,	m_glyphColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_glyphFilterColor(0.0f, 0.0f, 0.0f, 0.0f)
{
}

AccDisplayRenderer::~AccDisplayRenderer()
{
	T_EXCEPTION_GUARD_BEGIN

	destroy();

	T_EXCEPTION_GUARD_END
}

bool AccDisplayRenderer::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t frameCount,
	uint32_t renderContextSize,
	bool clearBackground,
	bool shapeCache,
	bool clipToDirtyRegion,
	float /*stereoscopicOffset*/
)
{
	m_resourceManager = resourceManager;
	m_renderSystem = renderSystem;
	m_gradientCache = new AccGradientCache(m_renderSystem);
	m_textureCache = new AccTextureCache(m_resourceManager, m_renderSystem);
	m_clearBackground = clearBackground;
	m_clipToDirtyRegion = clipToDirtyRegion;

	m_shapeResources = new AccShapeResources();
	if (!m_shapeResources->create(resourceManager))
	{
		log::error << L"Unable to create accelerated display renderer; failed to load shape resources" << Endl;
		return false;
	}

	m_vertexPool = new AccShapeVertexPool(renderSystem, frameCount > 0 ? frameCount : 1);
	if (!m_vertexPool->create())
	{
		log::error << L"Unable to create accelerated display renderer; failed to create vertex pool" << Endl;
		return false;
	}

	m_glyph = new AccGlyph();
	if (!m_glyph->create(resourceManager, renderSystem))
	{
		log::error << L"Unable to create accelerated display renderer; failed to create glyph list" << Endl;
		return false;
	}

	m_quad = new AccQuad();
	if (!m_quad->create(resourceManager, renderSystem))
	{
		log::error << L"Unable to create accelerated display renderer; failed to create quad shape" << Endl;
		return false;
	}

	render::RenderTargetSetCreateDesc rtscd;
	rtscd.count = 1;
	rtscd.width = c_cacheGlyphDimX;
	rtscd.height = c_cacheGlyphDimY;
	rtscd.multiSample = 0;
#if !defined(__PS3__)
	rtscd.createDepthStencil = false;
#else
	rtscd.createDepthStencil = true;	// RSX crash without depth buffer, why?
#endif
	rtscd.usingPrimaryDepthStencil = false;
	rtscd.targets[0].format = render::TfR8;

	m_renderTargetGlyphs = m_renderSystem->createRenderTargetSet(rtscd);
	if (!m_renderTargetGlyphs)
	{
		log::error << L"Unable to create accelerated display renderer; failed to create glyph cache target" << Endl;
		return false;
	}

	if (shapeCache)
	{
		m_shapeRenderer = new AccShapeRenderer();
		if (!m_shapeRenderer->create(renderSystem, resourceManager))
		{
			log::error << L"Unable to create accelerated display renderer; failed to shape renderer" << Endl;
			return false;
		}
	}

	m_renderContexts.resize(frameCount);
	for (uint32_t i = 0; i < frameCount; ++i)
		m_renderContexts[i] = new render::RenderContext(renderContextSize);

	return true;
}

void AccDisplayRenderer::destroy()
{
	m_renderSystem = 0;

	safeDestroy(m_glyph);
	safeDestroy(m_quad);
	safeDestroy(m_gradientCache);
	safeDestroy(m_textureCache);
	safeDestroy(m_renderTargetGlyphs);

	for (SmallMap< int32_t, ShapeCache >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
		safeDestroy(i->second.shape);

	for (SmallMap< int32_t, GlyphCache >::iterator i = m_glyphCache.begin(); i != m_glyphCache.end(); ++i)
		safeDestroy(i->second.shape);

	m_shapeCache.clear();
	m_glyphCache.clear();

	safeDestroy(m_shapeRenderer);
	safeDestroy(m_shapeResources);
	safeDestroy(m_vertexPool);

	m_renderContexts.clear();
	m_renderContext = 0;
}

void AccDisplayRenderer::build(uint32_t frame)
{
	m_renderContext = m_renderContexts[frame];
	m_renderContext->flush();

	m_frameTransform.set(0.0f, 0.0f, 1.0f, 1.0f);
}

void AccDisplayRenderer::build(render::RenderContext* renderContext, uint32_t frame)
{
	m_renderContext = renderContext;
}

void AccDisplayRenderer::render(render::IRenderView* renderView, uint32_t frame, render::EyeType eye, const Vector2& offset, float scale)
{
	T_RENDER_PUSH_MARKER(renderView, "Flash: Render");
	m_renderContexts[frame]->render(renderView, render::RpOverlay, 0);
	T_RENDER_POP_MARKER(renderView);
}

void AccDisplayRenderer::flush()
{
	for (RefArray< render::RenderContext >::iterator i = m_renderContexts.begin(); i != m_renderContexts.end(); ++i)
		(*i)->flush();
}

void AccDisplayRenderer::flushCaches()
{
	m_gradientCache->clear();
	m_textureCache->clear();

	for (SmallMap< int32_t, ShapeCache >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
		safeDestroy(i->second.shape);

	for (SmallMap< int32_t, GlyphCache >::iterator i = m_glyphCache.begin(); i != m_glyphCache.end(); ++i)
		safeDestroy(i->second.shape);

	m_shapeCache.clear();
	m_glyphCache.clear();

	m_nextIndex = 0;
}

bool AccDisplayRenderer::wantDirtyRegion() const
{
	return m_clipToDirtyRegion;
}

void AccDisplayRenderer::begin(
	const Dictionary& dictionary,
	const Color4f& backgroundColor,
	const Aabb2& frameBounds,
	const Vector4& frameTransform,
	float viewWidth,
	float viewHeight,
	const Aabb2& dirtyRegion
)
{
	bool viewSizeChanged = bool(viewWidth != m_viewSize.x() || viewHeight != m_viewSize.y());

	if (m_clipToDirtyRegion && (viewSizeChanged || !m_frameTarget))
	{
		safeDestroy(m_frameTarget);

		render::RenderTargetSetCreateDesc rtscd;
		rtscd.count = 1;
		rtscd.width = int32_t(viewWidth);
		rtscd.height = int32_t(viewHeight);
		rtscd.multiSample = 0;
		rtscd.createDepthStencil = true;
		rtscd.usingPrimaryDepthStencil = false;
		rtscd.ignoreStencil = false;
		rtscd.targets[0].format = render::TfR8G8B8A8;

		m_frameTarget = m_renderSystem->createRenderTargetSet(rtscd);
	}

	if (m_frameTarget)
	{
		render::TargetBeginRenderBlock* renderBlock = m_renderContext->alloc< render::TargetBeginRenderBlock >("Flash begin target");
		renderBlock->renderTargetSet = m_frameTarget;
		renderBlock->renderTargetIndex = 0;
		m_renderContext->draw(render::RpOverlay, renderBlock);
	}

	m_frameBounds.set(frameBounds.mn.x, frameBounds.mn.y, frameBounds.mx.x, frameBounds.mx.y);
	m_frameTransform = frameTransform;
	m_viewSize.set(viewWidth, viewHeight, 1.0f / viewWidth, 1.0f / viewHeight);

	const Vector2 Ft_offset(frameTransform.x(), frameTransform.y());
	const Vector2 Ft_scale(frameTransform.z(), frameTransform.w());
	m_frameBoundsVisible.mn = frameBounds.mn + (frameBounds.mx - frameBounds.mn) * (Ft_offset / Ft_scale);
	m_frameBoundsVisible.mx = frameBounds.mn + (frameBounds.mx - frameBounds.mn) * ((Vector2::one() - Ft_offset) / Ft_scale);

	if (m_clearBackground && !m_clipToDirtyRegion)
	{
		render::TargetClearRenderBlock* renderBlock = m_renderContext->alloc< render::TargetClearRenderBlock >("Flash clear (color+stencil)");
		renderBlock->clearMask = render::CfColor | render::CfStencil;
		renderBlock->clearColor = backgroundColor;
		m_renderContext->draw(render::RpOverlay, renderBlock);
	}
	else
	{
		render::TargetClearRenderBlock* renderBlock = m_renderContext->alloc< render::TargetClearRenderBlock >("Flash clear (stencil)");
		renderBlock->clearMask = render::CfStencil;
		m_renderContext->draw(render::RpOverlay, renderBlock);
	}

	// Flush glyph cache is RT has become invalid.
	if (!m_renderTargetGlyphs->isContentValid())
	{
		m_renderTargetGlyphs->setContentValid(true);
		for (SmallMap< int32_t, GlyphCache >::iterator i = m_glyphCache.begin(); i != m_glyphCache.end(); ++i)
			i->second.index = -1;
	}

	m_glyph->beginFrame();

	if (m_shapeRenderer)
		m_shapeRenderer->beginFrame();

	m_maskWrite = false;
	m_maskIncrement = false;
	m_maskReference = 0;

	// Setup stencil for dirty region.
	if (m_clipToDirtyRegion)
	{
		// If size of output view has changed then we disregard given dirty region as we need to clear entire frame.
		if (!viewSizeChanged)
			m_dirtyRegion = !dirtyRegion.empty() ? m_frameBoundsVisible.overlapped(dirtyRegion) : Aabb2();
		else
			m_dirtyRegion = m_frameBoundsVisible;

		if (!m_dirtyRegion.empty())
		{
			// Draw quad in stencil for dirty region.
			beginMask(true);
			renderQuad(Matrix33::identity(), m_dirtyRegion, c_cxfWhite);
			endMask();

			// Clear background by drawing a solid quad with given color; cannot clear as it doesn't handle stencil.
			if (m_clearBackground)
			{
				ColorTransform clearCxForm(Color4f(0.0f, 0.0f, 0.0f, 0.0f), backgroundColor.rgb1());
				renderQuad(Matrix33::identity(), m_dirtyRegion, clearCxForm);
			}
		}
	}
	else
	{
		// Dirty regions not used but set region to entire frame so we can
		// cull shapes trivially later.
		m_dirtyRegion = m_frameBoundsVisible;
	}
}

void AccDisplayRenderer::beginSprite(const SpriteInstance& sprite, const Matrix33& transform)
{
	if (m_shapeRenderer)
		m_shapeRenderer->beginSprite(
			m_renderContext,
			sprite,
			m_frameBounds,
			m_frameTransform,
			m_viewSize,
			transform,
			m_maskReference
		);
}

void AccDisplayRenderer::endSprite(const SpriteInstance& sprite, const Matrix33& transform)
{
	if (m_shapeRenderer)
		m_shapeRenderer->endSprite(
			m_renderContext,
			sprite,
			m_frameBounds,
			m_frameTransform,
			transform,
			m_maskReference
		);
}

void AccDisplayRenderer::beginEdit(const EditInstance& edit, const Matrix33& transform)
{
#if !defined(__ANDROID__)
	if (edit.getRenderClipMask())
	{
		beginMask(true);
		renderQuad(transform, edit.getTextBounds(), c_cxfWhite);
		endMask();
	}
#endif
}

void AccDisplayRenderer::endEdit(const EditInstance& edit, const Matrix33& transform)
{
#if !defined(__ANDROID__)
	if (edit.getRenderClipMask())
	{
		beginMask(false);
		renderQuad(transform, edit.getTextBounds(), c_cxfWhite);
		endMask();
	}
#endif
}

void AccDisplayRenderer::beginMask(bool increment)
{
	renderEnqueuedGlyphs();

	m_maskWrite = true;
	m_maskIncrement = increment;
}

void AccDisplayRenderer::endMask()
{
	renderEnqueuedGlyphs();

	m_maskWrite = false;
	if (m_maskIncrement)
	{
		T_ASSERT (m_maskReference < 255);
		m_maskReference++;
	}
	else
	{
		T_ASSERT (m_maskReference > 0);
		m_maskReference--;
	}
}

void AccDisplayRenderer::renderShape(const Dictionary& dictionary, const Matrix33& transform, const Shape& shape, const ColorTransform& cxform, uint8_t blendMode)
{
	Ref< AccShape > accShape;

	// Check if shape is within frame bounds, don't cull if we're in the middle of rendering cached bitmap.
	if (!m_shapeRenderer || m_shapeRenderer->shouldCull())
	{
		if (!rectangleVisible(m_dirtyRegion, transform * shape.getShapeBounds()))
			return;
	}

	// Get accelerated shape.
	int32_t tag = shape.getCacheTag();
	SmallMap< int32_t, ShapeCache >::iterator it = m_shapeCache.find(tag);
	if (it == m_shapeCache.end())
	{
		accShape = new AccShape(m_shapeResources);
		if (!accShape->createFromShape(
			m_vertexPool,
			m_gradientCache,
			m_textureCache,
			dictionary,
			shape
		))
			return;

		m_shapeCache[tag].unusedCount = 0;
		m_shapeCache[tag].shape = accShape;
	}
	else
	{
		it->second.unusedCount = 0;
		accShape = it->second.shape;
	}

	// Flush queued glyph shapes, must do this to ensure proper draw order.
	renderEnqueuedGlyphs();

	if (m_shapeRenderer)
	{
		// Render shape through shape cache.
		m_shapeRenderer->render(
			m_renderContext,
			accShape,
			tag,
			cxform,
			m_frameBounds,
			m_frameTransform,
			transform,
			m_maskWrite,
			m_maskIncrement,
			m_maskReference,
			blendMode
		);
	}
	else
	{
		// No shape cache; render directly.
		accShape->render(
			m_renderContext,
			transform,
			m_frameBounds,
			m_frameTransform,
			cxform,
			m_maskWrite,
			m_maskIncrement,
			m_maskReference,
			blendMode
		);
	}
}

void AccDisplayRenderer::renderMorphShape(const Dictionary& dictionary, const Matrix33& transform, const MorphShape& shape, const ColorTransform& cxform)
{
}

void AccDisplayRenderer::renderGlyph(
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
)
{
	// Only support embedded fonts.
	if (!glyph)
		return;

	float coordScale = font->getCoordinateType() == Font::CtTwips ? 1.0f / 1000.0f : 1.0f / (20.0f * 1000.0f);
	float fontScale = coordScale * fontHeight;
	Matrix33 glyphTransform = transform * scale(fontScale, fontScale);
	Color4f glyphColor = color * cxform.mul + cxform.add;

	// Check if shape is within frame bounds, don't cull if we're in the middle of rendering cached bitmap.
	if (!m_shapeRenderer || m_shapeRenderer->shouldCull())
	{
		if (!rectangleVisible(m_dirtyRegion, glyphTransform * glyph->getShapeBounds()))
			return;
	}

	if (m_glyphFilter != filter || !colorsEqual(glyphColor, m_glyphColor))
	{
		renderEnqueuedGlyphs();
		m_glyphFilter = filter;
		m_glyphColor = glyphColor;
		m_glyphFilterColor = filterColor;
	}

	uint32_t tag = glyph->getCacheTag();

	SmallMap< int32_t, GlyphCache >::iterator it1 = m_glyphCache.find(tag);
	if (it1 == m_glyphCache.end())
	{
		Ref< AccShape > accShape = new AccShape(m_shapeResources);
		if (!accShape->createFromGlyph(
			m_vertexPool,
			m_gradientCache,
			m_textureCache,
			dictionary,
			*glyph
		))
		{
			T_DEBUG(L"Glyph tesselation failed");
			return;
		}

		m_glyphCache[tag].shape = accShape;
		m_glyphCache[tag].index = -1;

		it1 = m_glyphCache.find(tag);
		T_ASSERT (it1 != m_glyphCache.end());
	}

	Ref< AccShape > accShape = it1->second.shape;
	T_ASSERT (accShape);

	float cachePixelDx = 1.0f / c_cacheGlyphDimX;
	float cachePixelDy = 1.0f / c_cacheGlyphDimY;

	// Always use maximum glyph bounds.
	Aabb2 bounds = accShape->getBounds();
	bounds.mx = bounds.mn + font->getMaxDimension();

	// Get cached glyph target.
	if (it1->second.index < 0)
	{
		// Glyph not cached; pick index by cycling which means oldest glyph get discarded.
		int32_t index = m_nextIndex++;
		if (m_nextIndex >= c_cacheGlyphCount)
			m_nextIndex = 0;

		for (SmallMap< int32_t, GlyphCache >::iterator i = m_glyphCache.begin(); i != m_glyphCache.end(); ++i)
		{
			if (i->second.index == index)
				i->second.index = -1;
		}

		int32_t column = index & (c_cacheGlyphCountX - 1);
		int32_t row = index / c_cacheGlyphCountX;

		Vector4 frameSize(bounds.mn.x, bounds.mn.y, bounds.mx.x, bounds.mx.y);
		Vector4 viewOffset(
			float(column) / c_cacheGlyphCountX,
			float(row) / c_cacheGlyphCountY,
			1.0f / c_cacheGlyphCountX,
			1.0f / c_cacheGlyphCountY
		);
		Vector4 viewOffsetWithMargin = viewOffset + Vector4(
			cachePixelDx * c_cacheGlyphMargin,
			cachePixelDy * c_cacheGlyphMargin,
			-cachePixelDx * c_cacheGlyphMargin * 2.0f,
			-cachePixelDy * c_cacheGlyphMargin * 2.0f
		);

		render::TargetBeginRenderBlock* renderBlockBegin = m_renderContext->alloc< render::TargetBeginRenderBlock >("Flash glyph render begin");
		renderBlockBegin->renderTargetSet = m_renderTargetGlyphs;
		renderBlockBegin->renderTargetIndex = 0;
		m_renderContext->draw(render::RpOverlay, renderBlockBegin);

		// Clear previous glyph by drawing a solid quad at it's place.
		m_quad->render(
			m_renderContext,
			bounds,
			Matrix33::identity(),
			frameSize,
			viewOffset,
			c_cxfZero,
			0,
			Vector4::zero(),
			false,
			false,
			0
		);

		accShape->render(
			m_renderContext,
			Matrix33::identity(),
			frameSize,
			viewOffsetWithMargin,
			c_cxfYellow,
			false,
			false,
			false,
			SbmDefault
		);

		render::TargetEndRenderBlock* renderBlockEnd = m_renderContext->alloc< render::TargetEndRenderBlock >("Flash glyph render end");
		m_renderContext->draw(render::RpOverlay, renderBlockEnd);

		it1->second.index = index;
	}

	int32_t column = it1->second.index & (c_cacheGlyphCountX - 1);
	int32_t row = it1->second.index / c_cacheGlyphCountX;

	m_glyph->add(
		bounds,
		glyphTransform,
		Vector4(
			float(column) / c_cacheGlyphCountX,
			float(row) / c_cacheGlyphCountY,
			1.0f / c_cacheGlyphCountX,
			1.0f / c_cacheGlyphCountY
		)
	);
}

void AccDisplayRenderer::renderQuad(const Matrix33& transform, const Aabb2& bounds, const ColorTransform& cxform)
{
	if (!m_shapeRenderer || m_shapeRenderer->shouldCull())
	{
		if (!rectangleVisible(m_dirtyRegion, transform * bounds))
			return;
	}

	m_quad->render(
		m_renderContext,
		bounds,
		transform,
		m_frameBounds,
		m_frameTransform,
		cxform,
		0,
		Vector4::zero(),
		m_maskWrite,
		m_maskIncrement,
		m_maskReference
	);
}

void AccDisplayRenderer::renderCanvas(const Matrix33& transform, const Canvas& canvas, const ColorTransform& cxform, uint8_t blendMode)
{
	Ref< AccShape > accShape;

	int32_t tag = canvas.getCacheTag();
	SmallMap< int32_t, ShapeCache >::iterator it = m_shapeCache.find(tag);
	if (it == m_shapeCache.end() || it->second.tag != canvas.getDirtyTag())
	{
		if (it != m_shapeCache.end() && it->second.shape)
		{
			it->second.shape->destroy();
			it->second.shape = 0;
		}

		accShape = new AccShape(m_shapeResources);
		if (!accShape->createFromCanvas(
			m_vertexPool,
			m_gradientCache,
			m_textureCache,
			canvas
		))
			return;

		m_shapeCache[tag].unusedCount = 0;
		m_shapeCache[tag].shape = accShape;
		m_shapeCache[tag].tag = canvas.getDirtyTag();
	}
	else
	{
		it->second.unusedCount = 0;
		accShape = it->second.shape;
	}

	if (!m_shapeRenderer || m_shapeRenderer->shouldCull())
	{
		if (!rectangleVisible(m_dirtyRegion, transform * accShape->getBounds()))
			return;
	}

	renderEnqueuedGlyphs();

		if (m_shapeRenderer)
	{
		// Render shape through shape cache.
		m_shapeRenderer->render(
			m_renderContext,
			accShape,
			tag,
			cxform,
			m_frameBounds,
			m_frameTransform,
			transform,
			m_maskWrite,
			m_maskIncrement,
			m_maskReference,
			blendMode
		);
	}
	else
	{
		// No shape cache; render directly.
		accShape->render(
			m_renderContext,
			transform,
			m_frameBounds,
			m_frameTransform,
			cxform,
			m_maskWrite,
			m_maskIncrement,
			m_maskReference,
			blendMode
		);
	}
}

void AccDisplayRenderer::end()
{
	renderEnqueuedGlyphs();

	if (m_shapeRenderer)
		m_shapeRenderer->endFrame();

	m_glyph->endFrame();

	// Clear dirty region to ensure stencil is reset properly.
	if (m_clipToDirtyRegion)
	{
		beginMask(false);
		renderQuad(Matrix33::identity(), m_dirtyRegion, c_cxfWhite);
		endMask();
	}

	// Don't flush cache if it doesn't contain that many shapes.
	if (m_shapeCache.size() >= c_maxCacheSize)
	{
		// Nuke cached shapes which hasn't been used for X number of frames.
		for (SmallMap< int32_t, ShapeCache >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); )
		{
			if (i->second.unusedCount++ >= c_maxUnusedCount)
			{
				if (i->second.shape)
					i->second.shape->destroy();
				i = m_shapeCache.erase(i);
			}
			else
				++i;
		}
	}
	else
	{
		// Increment "unused" counter still.
		for (SmallMap< int32_t, ShapeCache >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
			i->second.unusedCount++;
	}

	if (m_frameTarget)
	{
		render::TargetEndRenderBlock* renderBlock = m_renderContext->alloc< render::TargetEndRenderBlock >("Flash end target");
		m_renderContext->draw(render::RpOverlay, renderBlock);
		m_quad->blit(m_renderContext, m_frameTarget->getColorTexture(0));
	}

	m_gradientCache->synchronize();
	m_vertexPool->cycleGarbage();
	m_renderContext = 0;
}

void AccDisplayRenderer::renderEnqueuedGlyphs()
{
	m_glyph->render(
		m_renderContext,
		m_frameBounds,
		m_frameTransform,
		m_renderTargetGlyphs->getColorTexture(0),
		m_maskReference,
		m_glyphFilter,
		m_glyphColor,
		m_glyphFilterColor
	);
}

	}
}
