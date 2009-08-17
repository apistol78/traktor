#include <algorithm>
#include "AccDisplayRenderer.h"
#include "AccTextureCache.h"
#include "AccShape.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashFrame.h"
#include "Flash/FlashShape.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Core/Heap/GcNew.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const uint32_t c_maxCacheSize = 64;
const uint32_t c_maxUnusedCount = 40;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccDisplayRenderer", AccDisplayRenderer, DisplayRenderer)

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
	render::IRenderView* renderView,
	bool clearBackground
)
{
	m_resourceManager = resourceManager;
	m_renderSystem = renderSystem;
	m_renderView = renderView;
	m_textureCache = gc_new< AccTextureCache >(m_renderSystem);
	m_clearBackground = clearBackground;
	return true;
}

void AccDisplayRenderer::destroy()
{
	m_renderSystem = 0;
	m_renderView = 0;
	
	if (m_textureCache)
	{
		m_textureCache->destroy();
		m_textureCache = 0;
	}

	for (std::map< uint32_t, CacheEntry >::iterator i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
	{
		if (i->second.shape)
			i->second.shape->destroy();
	}
	m_shapeCache.clear();
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
		accShape = gc_new< AccShape >();
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
	
	Ref< AccShape > accShape;

	std::map< uint32_t, CacheEntry >::iterator it = m_shapeCache.find(hash);
	if (it == m_shapeCache.end())
	{
		accShape = gc_new< AccShape >();
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

	SwfCxTransform cxf =
	{
		{ (color.red * cxform.red[0]) / 255.0f + cxform.red[1], 0.0f, },
		{ (color.green * cxform.green[0]) / 255.0f + cxform.green[1], 0.0f },
		{ (color.blue * cxform.blue[0]) / 255.0f + cxform.blue[1], 0.0f },
		{ (color.alpha * cxform.alpha[0]) / 255.0f + cxform.alpha[1], 0.0f }
	};

	accShape->render(
		m_renderView,
		shape,
		m_frameSize,
		transform,
		cxf,
		m_maskWrite,
		m_maskIncrement,
		m_maskReference
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
		log::debug << L"Shape cache size " << m_shapeCache.size() << Endl;
#endif
}

	}
}
