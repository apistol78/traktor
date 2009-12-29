#include <algorithm>
#include "Core/Log/Log.h"
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

namespace traktor
{
	namespace flash
	{
		namespace
		{

const uint32_t c_maxCacheSize = 64;
const uint32_t c_maxUnusedCount = 40;
const uint32_t c_maxCachedGlyphs = 16;
const uint32_t c_cacheGlyphSize = 256;
const uint32_t c_cacheGlyphMargin = 4;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccDisplayRenderer", AccDisplayRenderer, IDisplayRenderer)

AccDisplayRenderer::AccDisplayRenderer()
:	m_frameSize(0.0f, 0.0f, 0.0f, 0.0f)
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
#if !defined(_PS3)
		rtscd.targets[0].format = render::TfR8;
#else
		rtscd.targets[0].format = render::TfR8G8B8A8;
#endif

		m_renderTargetGlyphs[i] = m_renderSystem->createRenderTargetSet(rtscd);
		if (!m_renderTargetGlyphs[i])
			return false;
	}

	return true;
}

void AccDisplayRenderer::destroy()
{
	T_ASSERT (m_renderView == 0);
	m_renderSystem = 0;
	
	safeDestroy(m_quad);
	safeDestroy(m_textureCache);

	for (RefArray< render::RenderTargetSet >::iterator i = m_renderTargetGlyphs.begin(); i != m_renderTargetGlyphs.end(); ++i)
		(*i)->destroy();

	m_renderTargetGlyphs.clear();

	for (std::map< uint32_t, CacheEntry >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
		safeDestroy(i->second.shape);

	m_shapeCache.clear();
}

void AccDisplayRenderer::beginRender(render::IRenderView* renderView)
{
	T_ASSERT (m_renderView == 0);
	m_renderView = renderView;
}

void AccDisplayRenderer::endRender()
{
	m_renderView = 0;
}

void AccDisplayRenderer::begin(const FlashMovie& movie, const SwfColor& backgroundColor)
{
	T_ASSERT (m_renderView != 0);
	const SwfRect& bounds = movie.getFrameBounds();

	m_frameSize = Vector4(
		bounds.min.x,
		bounds.min.y,
		bounds.max.x,
		bounds.max.y
	);

	const float clearColor[] =
	{
		backgroundColor.red / 255.0f,
		backgroundColor.green / 255.0f,
		backgroundColor.blue / 255.0f,
		0.0f
	};

	if (m_clearBackground)
	{
		m_renderView->clear(
			render::CfColor | render::CfStencil,
			clearColor,
			0.0f,
			0
		);
	}
	else
	{
		m_renderView->clear(
			render::CfStencil,
			clearColor,
			0.0f,
			0
		);
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
	uint32_t hash = reinterpret_cast< uint32_t >(&shape);
	
	Ref< AccShape > accShape;

	std::map< uint32_t, CacheEntry >::iterator it = m_shapeCache.find(hash);
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
		m_renderView,
		shape,
		m_frameSize,
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
	uint32_t hash = reinterpret_cast< uint32_t >(&shape);
	Ref< render::RenderTargetSet > rts;
	Ref< AccShape > accShape;

	// Get glyph shape; create if not already cached.
	std::map< uint32_t, CacheEntry >::iterator it1 = m_shapeCache.find(hash);
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

	float px = c_cacheGlyphMargin * (bounds.max.x - bounds.min.x) / c_cacheGlyphSize;
	float py = c_cacheGlyphMargin * (bounds.max.y - bounds.min.y) / c_cacheGlyphSize;

	bounds.min.x -= px;
	bounds.min.y -= py;
	bounds.max.x += px;
	bounds.max.y += py;

	// Get cached glyph target.
	std::map< uint32_t, render::RenderTargetSet* >::iterator it2 = m_glyphCache.find(hash);
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
		for (std::map< uint32_t, render::RenderTargetSet* >::iterator i = m_glyphCache.begin(); i != m_glyphCache.end(); ++i)
		{
			if (i->second == rts)
			{
				m_glyphCache.erase(i);
				break;
			}
		}

		// Update render target with new glyph.
		m_renderView->begin(rts, 0, false);

		const float c_clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_renderView->clear(render::CfColor, c_clearColor, 1.0f, 0);

		const SwfCxTransform cxfi = { 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };

		accShape->render(
			m_renderView,
			shape,
			Vector4(
				bounds.min.x,
				bounds.min.y,
				bounds.max.x,
				bounds.max.y
			),
			Matrix33::identity(),
			cxfi,
			false,
			false,
			false
		);

		m_renderView->end();

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
		m_renderView,
		m_frameSize,
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
		for (std::map< uint32_t, CacheEntry >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
			i->second.unusedCount++;
		return;
	}

	// Nuke cached shapes which hasn't been used for X number of frames.
	for (std::map< uint32_t, CacheEntry >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); )
	{
		if (i->second.unusedCount++ >= c_maxUnusedCount)
		{
			i->second.shape->destroy();
			m_shapeCache.erase(i++);
		}
		else
			++i;
	}

#if defined(_DEBUG)
	static int s_frame = 0;
	if (s_frame++ % 100 == 0)
		log::debug << L"Shape cache size " << uint32_t(m_shapeCache.size()) << Endl;
#endif
}

	}
}
