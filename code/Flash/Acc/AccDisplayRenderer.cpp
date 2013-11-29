#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/FlashBitmap.h"
#include "Flash/FlashCanvas.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashShape.h"
#include "Flash/Acc/AccDisplayRenderer.h"
#include "Flash/Acc/AccGlyph.h"
#include "Flash/Acc/AccTextureCache.h"
#include "Flash/Acc/AccShape.h"
#include "Flash/Acc/AccShapeResources.h"
#include "Flash/Acc/AccShapeVertexPool.h"
#include "Flash/Acc/AccQuad.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/Context/RenderContext.h"

#define T_DEBUG_GLYPH_CACHE 0
#define T_FLUSH_CACHE 0

namespace traktor
{
	namespace flash
	{
		namespace
		{

#if T_FLUSH_CACHE
const uint32_t c_maxCacheSize = 64;
const uint32_t c_maxUnusedCount = 40;
#endif
#if TARGET_OS_IPHONE
const uint32_t c_cacheGlyphSize = 32;
#else
const uint32_t c_cacheGlyphSize = 128;
#endif
const uint32_t c_cacheGlyphMargin = 0;
const uint32_t c_cacheGlyphCountX = 16;
const uint32_t c_cacheGlyphCountY = 8;
const uint32_t c_cacheGlyphCount = c_cacheGlyphCountX * c_cacheGlyphCountY;
const uint32_t c_cacheGlyphDimX = c_cacheGlyphSize * c_cacheGlyphCountX;
const uint32_t c_cacheGlyphDimY = c_cacheGlyphSize * c_cacheGlyphCountY;

const SwfCxTransform c_cxfZero = { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f } };
const SwfCxTransform c_cxfYellow = { { 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } };
const SwfCxTransform c_cxfGreen = { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } };
const SwfCxTransform c_cxfIdentity = { { 1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f } };

bool rectangleVisible(
	const Vector4& frameSize,
	const Vector4& viewSize,
	const Vector4& viewOffset,
	const Matrix33& transform,
	const Aabb2& bounds
)
{
	if (bounds.mx.x <= bounds.mn.x || bounds.mx.y <= bounds.mn.y)
		return false;

	// Transform rectangle into frame.
	Vector2 emn = transform * bounds.mn;
	Vector2 emx = transform * bounds.mx;
	Vector2 xmn(min(emn.x, emx.x), min(emn.y, emx.y));
	Vector2 xmx(max(emn.x, emx.x), max(emn.y, emx.y));

	// Transform from frame into normalized coordinates.
	xmn.x = (xmn.x - frameSize.x()) / (frameSize.z() - frameSize.x());
	xmn.y = (xmn.y - frameSize.y()) / (frameSize.w() - frameSize.y());
	xmx.x = (xmx.x - frameSize.x()) / (frameSize.z() - frameSize.x());
	xmx.y = (xmx.y - frameSize.y()) / (frameSize.w() - frameSize.y());

	// Scale into view.
	xmn.x = (xmn.x * viewOffset.z()) + viewOffset.x();
	xmn.y = (xmn.y * viewOffset.w()) + viewOffset.y();
	xmx.x = (xmx.x * viewOffset.z()) + viewOffset.x();
	xmx.y = (xmx.y * viewOffset.w()) + viewOffset.y();

	// Check if extents are outside view rectangle.
	if (xmn.x > 1.0f || xmn.y > 1.0f)
		return false;
	if (xmx.x < 0.0f || xmx.y < 0.0f)
		return false;

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccDisplayRenderer", AccDisplayRenderer, IDisplayRenderer)

AccDisplayRenderer::AccDisplayRenderer()
:	m_vertexPool(0)
,	m_nextIndex(0)
,	m_frameSize(0.0f, 0.0f, 0.0f, 0.0f)
,	m_viewSize(0.0f, 0.0f, 0.0f, 0.0f)
,	m_viewOffset(0.0f, 0.0f, 1.0f, 1.0f)
,	m_clearBackground(false)
,	m_stereoscopicOffset(0.0f)
,	m_maskWrite(false)
,	m_maskIncrement(false)
,	m_maskReference(0)
,	m_glyphFilter(0)
,	m_handleScreenOffset(render::getParameterHandle(L"Flash_ScreenOffset"))
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
	bool clearBackground,
	float stereoscopicOffset
)
{
	m_resourceManager = resourceManager;
	m_renderSystem = renderSystem;
	m_textureCache = new AccTextureCache(m_renderSystem);
	m_clearBackground = clearBackground;
	m_stereoscopicOffset = stereoscopicOffset;

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
	rtscd.createDepthStencil = false;
	rtscd.usingPrimaryDepthStencil = false;
	rtscd.targets[0].format = render::TfR8;

	m_renderTargetGlyphs = m_renderSystem->createRenderTargetSet(rtscd);
	if (!m_renderTargetGlyphs)
	{
		log::error << L"Unable to create accelerated display renderer; failed to create glyph cache target" << Endl;
		return false;
	}

	m_renderContexts.resize(frameCount);
	for (uint32_t i = 0; i < frameCount; ++i)
		m_renderContexts[i] = new render::RenderContext(1024 * 1024);

	// Allocate "global" parameter context; as it's reset for each render
	// call this can be fairly small.
	m_globalContext = new render::RenderContext(4096);

	return true;
}

void AccDisplayRenderer::destroy()
{
	m_renderSystem = 0;

	safeDestroy(m_glyph);
	safeDestroy(m_quad);
	safeDestroy(m_textureCache);
	safeDestroy(m_renderTargetGlyphs);

	for (SmallMap< int32_t, ShapeCache >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
		safeDestroy(i->second.shape);

	for (SmallMap< int32_t, GlyphCache >::iterator i = m_glyphCache.begin(); i != m_glyphCache.end(); ++i)
		safeDestroy(i->second.shape);

	m_shapeCache.clear();
	m_glyphCache.clear();

	safeDestroy(m_shapeResources);
	safeDestroy(m_vertexPool);

	m_renderContexts.clear();
	m_renderContext = 0;

	m_globalContext = 0;
}

void AccDisplayRenderer::build(uint32_t frame)
{
	m_renderContext = m_renderContexts[frame];
	m_renderContext->flush();

	m_viewOffset.set(0.0f, 0.0f, 1.0f, 1.0f);

	for (SmallMap< int32_t, ShapeCache >::const_iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
		i->second.shape->preBuild();
	for (SmallMap< int32_t, GlyphCache >::iterator i = m_glyphCache.begin(); i != m_glyphCache.end(); ++i)
		i->second.shape->preBuild();
}

void AccDisplayRenderer::build(render::RenderContext* renderContext, uint32_t frame)
{
	m_renderContext = renderContext;

	for (SmallMap< int32_t, ShapeCache >::const_iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
		i->second.shape->preBuild();
	for (SmallMap< int32_t, GlyphCache >::iterator i = m_glyphCache.begin(); i != m_glyphCache.end(); ++i)
		i->second.shape->preBuild();
}

void AccDisplayRenderer::render(render::IRenderView* renderView, uint32_t frame, render::EyeType eye, const Vector2& offset, float scale)
{
	render::ProgramParameters programParams;
	programParams.beginParameters(m_globalContext);

	Vector4 screenOffset;
	if (eye == render::EtCyclop)
		screenOffset = Vector4(offset.x, offset.y, scale, scale);
	else if (eye == render::EtLeft)
		screenOffset = Vector4(offset.x - m_stereoscopicOffset, offset.y, scale, scale);
	else if (eye == render::EtRight)
		screenOffset = Vector4(offset.x + m_stereoscopicOffset, offset.y, scale, scale);

	programParams.setVectorParameter(m_handleScreenOffset, screenOffset);
	programParams.endParameters(m_globalContext);

	m_renderContexts[frame]->render(renderView, render::RpOverlay, &programParams);

	m_globalContext->flush();
}

void AccDisplayRenderer::begin(
	const FlashDictionary& dictionary,
	const SwfColor& backgroundColor,
	const Aabb2& frameBounds,
	float viewWidth,
	float viewHeight,
	const Vector4& viewOffset
)
{
	m_frameSize.set(frameBounds.mn.x, frameBounds.mn.y, frameBounds.mx.x, frameBounds.mx.y);
	m_viewSize.set(viewWidth, viewHeight, 1.0f / viewWidth, 1.0f / viewHeight);
	m_viewOffset = viewOffset;

	if (m_clearBackground)
	{
		render::TargetClearRenderBlock* renderBlock = m_renderContext->alloc< render::TargetClearRenderBlock >("Flash clear (color+stencil)");
		renderBlock->clearMask = render::CfColor | render::CfStencil;
		renderBlock->clearColor.set(
			backgroundColor.red / 255.0f,
			backgroundColor.green / 255.0f,
			backgroundColor.blue / 255.0f
		);
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

	m_maskWrite = false;
	m_maskIncrement = false;
	m_maskReference = 0;
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

void AccDisplayRenderer::renderShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashShape& shape, const SwfCxTransform& cxform)
{
	Ref< AccShape > accShape;

	int32_t tag = shape.getCacheTag();
	SmallMap< int32_t, ShapeCache >::iterator it = m_shapeCache.find(tag);
	if (it == m_shapeCache.end())
	{
		accShape = new AccShape(m_shapeResources);
		if (!accShape->createTesselation(shape))
			return;

		m_shapeCache[tag].unusedCount = 0;
		m_shapeCache[tag].shape = accShape;
	}
	else
	{
		it->second.unusedCount = 0;
		accShape = it->second.shape;
	}

	if (!accShape->updateRenderable(
		m_vertexPool,
		m_textureCache,
		dictionary,
		shape.getFillStyles(),
		shape.getLineStyles()
	))
		return;

	if (!rectangleVisible(m_frameSize, m_viewSize, m_viewOffset, transform, accShape->getBounds()))
		return;

	renderEnqueuedGlyphs();

	accShape->render(
		m_renderContext,
		transform,
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		1.0f,
		cxform,
		m_maskWrite,
		m_maskIncrement,
		m_maskReference
	);
}

void AccDisplayRenderer::renderMorphShape(const FlashDictionary& dictionary, const Matrix33& transform, const FlashMorphShape& shape, const SwfCxTransform& cxform)
{
}

void AccDisplayRenderer::renderGlyph(const FlashDictionary& dictionary, const Matrix33& transform, const Vector2& fontMaxDimension, const FlashShape& shape, const SwfColor& color, const SwfCxTransform& cxform, uint8_t filter, const SwfColor& filterColor)
{
	if (m_glyphFilter != filter)
	{
		renderEnqueuedGlyphs();
		m_glyphFilter = filter;
		m_glyphFilterColor = filterColor;
	}

	uint32_t tag = shape.getCacheTag();

	SmallMap< int32_t, GlyphCache >::iterator it1 = m_glyphCache.find(tag);
	if (it1 == m_glyphCache.end())
	{
		Ref< AccShape > accShape = new AccShape(m_shapeResources);
		if (!accShape->createTesselation(shape))
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

	if (!accShape->updateRenderable(
		m_vertexPool,
		0,
		dictionary,
		shape.getFillStyles(),
		shape.getLineStyles()
	))
	{
		it1->second.index = -1;
		return;
	}

	Aabb2 bounds = accShape->getBounds();
	if (!rectangleVisible(m_frameSize, m_viewSize, m_viewOffset, transform, bounds))
		return;

	float cachePixelDx = 1.0f / c_cacheGlyphDimX;
	float cachePixelDy = 1.0f / c_cacheGlyphDimY;

	// Always use maximum glyph bounds.
	bounds.mx = bounds.mn + fontMaxDimension;

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
		Vector4 viewSize(0.0f, 0.0f, 0.0f, 0.0f);
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
			viewSize,
			viewOffset,
			0.0f,
			c_cxfZero,
			0,
			Vector4::zero(),
			0
		);

		accShape->render(
			m_renderContext,
			Matrix33::identity(),
			frameSize,
			viewSize,
			viewOffsetWithMargin,
			0.0f,
			c_cxfYellow,
			false,
			false,
			false
		);

		render::TargetEndRenderBlock* renderBlockEnd = m_renderContext->alloc< render::TargetEndRenderBlock >("Flash glyph render end");
		m_renderContext->draw(render::RpOverlay, renderBlockEnd);

		it1->second.index = index;
	}

	// Draw glyph quad.
	SwfCxTransform cxf =
	{
		{ (color.red * cxform.red[0]) / 255.0f + cxform.red[1], 0.0f, },
		{ (color.green * cxform.green[0]) / 255.0f + cxform.green[1], 0.0f },
		{ (color.blue * cxform.blue[0]) / 255.0f + cxform.blue[1], 0.0f },
		{ (color.alpha * cxform.alpha[0]) / 255.0f + cxform.alpha[1], 0.0f }
	};

	int32_t column = it1->second.index & (c_cacheGlyphCountX - 1);
	int32_t row = it1->second.index / c_cacheGlyphCountX;

	m_glyph->add(
		bounds,
		transform,
		cxf,
		Vector4(
			float(column) / c_cacheGlyphCountX,
			float(row) / c_cacheGlyphCountY,
			1.0f / c_cacheGlyphCountX,
			1.0f / c_cacheGlyphCountY
		)
	);
}

void AccDisplayRenderer::renderCaret(const Matrix33& transform, const Vector2& fontMaxDimension, const SwfCxTransform& cxform)
{
	Aabb2 bounds;
	bounds.mn = -fontMaxDimension / 2.0f;
	bounds.mx = fontMaxDimension / 2.0f;

	m_quad->render(
		m_renderContext,
		bounds,
		transform,
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		0.0f,
		cxform,
		0,
		Vector4::zero(),
		m_maskReference
	);
}

void AccDisplayRenderer::renderCanvas(const FlashDictionary& dictionary, const Matrix33& transform, const FlashCanvas& canvas, const SwfCxTransform& cxform)
{
	Ref< AccShape > accShape;

	int32_t tag = canvas.getCacheTag();
	SmallMap< int32_t, ShapeCache >::iterator it = m_shapeCache.find(tag);
	if (it == m_shapeCache.end() || it->second.tag != canvas.getDirtyTag())
	{
		accShape = new AccShape(m_shapeResources);
		if (!accShape->createTesselation(canvas))
			return;

		m_shapeCache[tag].unusedCount = 0;
		m_shapeCache[tag].shape = accShape;
	}
	else
	{
		it->second.unusedCount = 0;
		accShape = it->second.shape;
	}

	if (!accShape->updateRenderable(
		m_vertexPool,
		m_textureCache,
		dictionary,
		canvas.getFillStyles(),
		canvas.getLineStyles()
	))
		return;

	if (!rectangleVisible(m_frameSize, m_viewSize, m_viewOffset, transform, accShape->getBounds()))
		return;

	renderEnqueuedGlyphs();

	accShape->render(
		m_renderContext,
		transform,
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		1.0f,
		cxform,
		m_maskWrite,
		m_maskIncrement,
		m_maskReference
	);
}

void AccDisplayRenderer::end()
{
	renderEnqueuedGlyphs();

#if T_DEBUG_GLYPH_CACHE
	// Overlay glyph cache.
	const Aabb2 bounds = { Vector2(0.0f, 0.0f), Vector2(c_cacheGlyphCountX * 1024, c_cacheGlyphCountY * 1024) };
	m_quad->render(
		m_renderContext,
		bounds,
		Matrix33::identity(),
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		0.0f,
		c_cxfIdentity,
		m_renderTargetGlyphs->getColorTexture(0),
		Vector4(0.0f, 0.0f, 1.0f, 1.0f),
		0
	);
#endif

#if T_FLUSH_CACHE
	// Don't flush cache if it doesn't contain that many shapes.
	if (m_shapeCache.size() < c_maxCacheSize)
	{
		// Increment "unused" counter still.
		for (std::map< uint64_t, ShapeCache >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
			i->second.unusedCount++;
		return;
	}

	// Nuke cached shapes which hasn't been used for X number of frames.
	for (SmallMap< int32_t, ShapeCache >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); )
	{
		if (i->second.unusedCount++ >= c_maxUnusedCount)
		{
			i->second.shape->destroy();
			m_shapeCache.erase(i++);
		}
		else
			++i;
	}
#endif

	m_vertexPool->cycleGarbage();

	m_renderContext = 0;
}

void AccDisplayRenderer::renderEnqueuedGlyphs()
{
	m_glyph->render(
		m_renderContext,
		m_frameSize,
		m_viewSize,
		m_viewOffset,
		1.0f,
		m_renderTargetGlyphs->getColorTexture(0),
		m_maskReference,
		m_glyphFilter,
		m_glyphFilterColor
	);
}

	}
}
