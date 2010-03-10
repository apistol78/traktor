#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashShape.h"
#include "Flash/Acc/AccDisplayRenderer.h"
#include "Flash/Acc/AccTextureCache.h"
#include "Flash/Acc/AccShape.h"
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

const uint32_t c_maxCacheSize = 64;
const uint32_t c_maxUnusedCount = 40;
const uint32_t c_maxCachedGlyphs = 32;
#if !defined(TARGET_OS_IPHONE)
const uint32_t c_cacheGlyphSize = 128;
#else
const uint32_t c_cacheGlyphSize = 32;
#endif
const uint32_t c_cacheGlyphMargin = 4;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccDisplayRenderer", AccDisplayRenderer, IDisplayRenderer)

AccDisplayRenderer::AccDisplayRenderer()
:	m_frameSize(0.0f, 0.0f, 0.0f, 0.0f)
,	m_viewSize(0.0f, 0.0f, 0.0f, 0.0f)
,	m_aspectRatio(1.0f)
,	m_scaleX(1.0f)
,	m_clearBackground(false)
,	m_maskWrite(false)
,	m_maskIncrement(false)
,	m_maskReference(0)
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
	bool clearBackground
)
{
	m_resourceManager = resourceManager;
	m_renderSystem = renderSystem;
	m_textureCache = new AccTextureCache(m_renderSystem);
	m_clearBackground = clearBackground;

	m_quad = new AccQuad();
	if (!m_quad->create(resourceManager, renderSystem))
		return false;

	m_renderTargetGlyphs.resize(c_maxCachedGlyphs);
	for (uint32_t i = 0; i < c_maxCachedGlyphs; ++i)
	{
		render::RenderTargetSetCreateDesc rtscd;

		rtscd.count = 1;
		rtscd.width = c_cacheGlyphSize;
		rtscd.height = c_cacheGlyphSize;
		rtscd.multiSample = 0;
		rtscd.depthStencil = false;
#if !defined(_PS3) && !defined(TARGET_OS_IPHONE)
		rtscd.targets[0].format = render::TfR8;
#else
		rtscd.targets[0].format = render::TfR8G8B8A8;
#endif

		m_renderTargetGlyphs[i] = m_renderSystem->createRenderTargetSet(rtscd);
		if (!m_renderTargetGlyphs[i])
			return false;
	}

	m_renderContexts.resize(frameCount);
	for (uint32_t i = 0; i < frameCount; ++i)
		m_renderContexts[i] = new render::RenderContext(256 * 1024);

	return true;
}

void AccDisplayRenderer::destroy()
{
	m_renderSystem = 0;

	safeDestroy(m_quad);
	safeDestroy(m_textureCache);

	for (RefArray< render::RenderTargetSet >::iterator i = m_renderTargetGlyphs.begin(); i != m_renderTargetGlyphs.end(); ++i)
		(*i)->destroy();
	m_renderTargetGlyphs.clear();

	for (std::map< uint64_t, CacheEntry >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
		safeDestroy(i->second.shape);
	m_shapeCache.clear();

	m_renderContexts.clear();
	m_renderContext = 0;
}

void AccDisplayRenderer::build(uint32_t frame, bool correctAspectRatio)
{
	m_renderContext = m_renderContexts[frame];

	if (correctAspectRatio)
		m_aspectRatio = m_viewSize.x() / m_viewSize.y();
	else
		m_aspectRatio = 0.0f;

	m_scaleX = 1.0f;
}

void AccDisplayRenderer::render(render::IRenderView* renderView, uint32_t frame)
{
	m_renderContexts[frame]->render(renderView, render::RfOverlay);

	// \fixme Not very nice.
	render::Viewport viewport = renderView->getViewport();
	m_viewSize.set(float(viewport.width), float(viewport.height), 1.0f / viewport.width, 1.0f / viewport.height);
}

void AccDisplayRenderer::flush(uint32_t frame)
{
	m_renderContexts[frame]->flush();
}

void AccDisplayRenderer::begin(const FlashMovie& movie, const SwfColor& backgroundColor)
{
	const SwfRect& bounds = movie.getFrameBounds();

	m_frameSize = Vector4(
		bounds.min.x,
		bounds.min.y,
		bounds.max.x,
		bounds.max.y
	);

	// Calculate horizontal scale factor to match aspect ratios.
	if (m_aspectRatio > FUZZY_EPSILON)
	{
		float frameAspect = (bounds.max.x - bounds.min.x) / (bounds.max.y - bounds.min.y);
		m_scaleX = frameAspect / m_aspectRatio;
	}

	if (m_clearBackground)
	{
		render::TargetClearRenderBlock* renderBlock = m_renderContext->alloc< render::TargetClearRenderBlock >();
		renderBlock->clearMask = render::CfColor | render::CfStencil;
		renderBlock->clearColor[0] = backgroundColor.red / 255.0f;
		renderBlock->clearColor[1] = backgroundColor.green / 255.0f;
		renderBlock->clearColor[2] = backgroundColor.blue / 255.0f;
		m_renderContext->draw(render::RfOverlay, renderBlock);
	}
	else
	{
		render::TargetClearRenderBlock* renderBlock = m_renderContext->alloc< render::TargetClearRenderBlock >();
		renderBlock->clearMask = render::CfStencil;
		m_renderContext->draw(render::RfOverlay, renderBlock);
	}

	m_maskWrite = false;
	m_maskIncrement = false;
	m_maskReference = 0;
}

void AccDisplayRenderer::beginMask(bool increment)
{
	m_maskWrite = true;
	m_maskIncrement = increment;
}

void AccDisplayRenderer::endMask()
{
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

void AccDisplayRenderer::renderShape(const FlashMovie& movie, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform)
{
	uint64_t hash = reinterpret_cast< uint64_t >(&shape);

	Ref< AccShape > accShape;

	std::map< uint64_t, CacheEntry >::iterator it = m_shapeCache.find(hash);
	if (it == m_shapeCache.end())
	{
		accShape = new AccShape();
		accShape->create(
			m_resourceManager,
			m_renderSystem,
			*m_textureCache,
			movie,
			shape
		);
		m_shapeCache[hash].unusedCount = 0;
		m_shapeCache[hash].shape = accShape;
	}
	else
	{
		it->second.unusedCount = 0;
		accShape = it->second.shape;
	}

	accShape->render(
		m_renderContext,
		shape,
		m_frameSize,
		m_viewSize,
		m_scaleX,
		transform,
		cxform,
		m_maskWrite,
		m_maskIncrement,
		m_maskReference
	);
}

void AccDisplayRenderer::renderMorphShape(const FlashMovie& movie, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform)
{
}

void AccDisplayRenderer::renderGlyph(const FlashMovie& movie, const Matrix33& transform, const FlashShape& shape, const SwfColor& color, const SwfCxTransform& cxform)
{
	uint64_t hash = reinterpret_cast< uint64_t >(&shape);
	Ref< render::RenderTargetSet > rts;
	Ref< AccShape > accShape;

	// Get glyph shape; create if not already cached.
	std::map< uint64_t, CacheEntry >::iterator it1 = m_shapeCache.find(hash);
	if (it1 == m_shapeCache.end())
	{
		accShape = new AccShape();
		accShape->create(
			m_resourceManager,
			m_renderSystem,
			*m_textureCache,
			movie,
			shape
		);
		m_shapeCache[hash].unusedCount = 0;
		m_shapeCache[hash].shape = accShape;
	}
	else
	{
		it1->second.unusedCount = 0;
		accShape = it1->second.shape;
	}

	// Calculate bounds with margin.
	SwfRect bounds = accShape->getBounds();
	if (bounds.max.x <= bounds.min.x || bounds.max.y <= bounds.min.y)
		return;

	float px = c_cacheGlyphMargin * (bounds.max.x - bounds.min.x) / c_cacheGlyphSize;
	float py = c_cacheGlyphMargin * (bounds.max.y - bounds.min.y) / c_cacheGlyphSize;

	bounds.min.x -= px;
	bounds.min.y -= py;
	bounds.max.x += px;
	bounds.max.y += py;

	// Get cached glyph target.
	std::map< uint64_t, render::RenderTargetSet* >::iterator it2 = m_glyphCache.find(hash);
	if (it2 != m_glyphCache.end())
	{
		rts = it2->second;
		T_ASSERT (rts);
	}
	else
	{
		// Glyph not cached; pick oldest render target which is done by simply
		// cycling the array.
		rts = m_renderTargetGlyphs.back();
		m_renderTargetGlyphs.pop_back();
		m_renderTargetGlyphs.push_front(rts);

		// Remove from cache; we're about to change target.
		for (std::map< uint64_t, render::RenderTargetSet* >::iterator i = m_glyphCache.begin(); i != m_glyphCache.end(); ++i)
		{
			if (i->second == rts)
			{
				m_glyphCache.erase(i);
				break;
			}
		}

		// Indicate target as valid here; we don't want to wait until
		// target actually been updated.
		rts->setContentValid(true);

		render::TargetBeginRenderBlock* renderBlockBegin = m_renderContext->alloc< render::TargetBeginRenderBlock >();
		renderBlockBegin->renderTargetSet = rts;
		renderBlockBegin->renderTargetIndex = 0;
		renderBlockBegin->keepDepthStencil = false;
		m_renderContext->draw(render::RfOverlay, renderBlockBegin);

		render::TargetClearRenderBlock* renderBlockClear = m_renderContext->alloc< render::TargetClearRenderBlock >();
		renderBlockClear->clearMask = render::CfColor;
		m_renderContext->draw(render::RfOverlay, renderBlockClear);

		const SwfCxTransform cxfi = { 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
		accShape->render(
			m_renderContext,
			shape,
			Vector4(
				bounds.min.x,
				bounds.min.y,
				bounds.max.x,
				bounds.max.y
			),
			m_viewSize,
			1.0f,
			Matrix33::identity(),
			cxfi,
			false,
			false,
			false
		);

		render::TargetEndRenderBlock* renderBlockEnd = m_renderContext->alloc< render::TargetEndRenderBlock >();
		m_renderContext->draw(render::RfOverlay, renderBlockEnd);

		// Place in cache.
		m_glyphCache[hash] = rts;
	}

	// Draw glyph quad.
	SwfCxTransform cxf =
	{
		{ (color.red * cxform.red[0]) / 255.0f + cxform.red[1], 0.0f, },
		{ (color.green * cxform.green[0]) / 255.0f + cxform.green[1], 0.0f },
		{ (color.blue * cxform.blue[0]) / 255.0f + cxform.blue[1], 0.0f },
		{ (color.alpha * cxform.alpha[0]) / 255.0f + cxform.alpha[1], 0.0f }
	};

	m_quad->render(
		m_renderContext,
		m_frameSize,
		m_viewSize,
		m_scaleX,
		transform,
		bounds,
		cxf,
		rts->getColorTexture(0)
	);
}

void AccDisplayRenderer::end()
{
	// Don't flush cache if it doesn't contain that many shapes.
	if (m_shapeCache.size() < c_maxCacheSize)
	{
		// Increment "unused" counter still.
		for (std::map< uint64_t, CacheEntry >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
			i->second.unusedCount++;
		return;
	}

	// Nuke cached shapes which hasn't been used for X number of frames.
	for (std::map< uint64_t, CacheEntry >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); )
	{
		if (i->second.unusedCount++ >= c_maxUnusedCount)
		{
			i->second.shape->destroy();
			m_shapeCache.erase(i++);
		}
		else
			++i;
	}

	// Remove cached glyphs if the target has become invalid.
	for (std::map< uint64_t, render::RenderTargetSet* >::iterator i = m_glyphCache.begin(); i != m_glyphCache.end(); )
	{
		if (!i->second->isContentValid())
			m_glyphCache.erase(i++);
		else
			++i;
	}
}

	}
}
