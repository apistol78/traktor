/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/Acc/AccDisplayRenderer.h"

#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Spark/Acc/AccCachedSprite.h"
#include "Spark/Acc/AccGlyph.h"
#include "Spark/Acc/AccGradientCache.h"
#include "Spark/Acc/AccQuad.h"
#include "Spark/Acc/AccShape.h"
#include "Spark/Acc/AccShapeResources.h"
#include "Spark/Acc/AccShapeVertexPool.h"
#include "Spark/Acc/AccTextureCache.h"
#include "Spark/Bitmap.h"
#include "Spark/Canvas.h"
#include "Spark/Dictionary.h"
#include "Spark/EditInstance.h"
#include "Spark/Font.h"
#include "Spark/Frame.h"
#include "Spark/Movie.h"
#include "Spark/Packer.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace traktor::spark
{
namespace
{

const uint32_t c_maxCacheSize = 32;
const uint32_t c_maxUnusedCount = 10;
const uint32_t c_cacheGlyphSize = 128;
const uint32_t c_cacheGlyphMargin = 1;
const uint32_t c_cacheGlyphCountX = 16;
const uint32_t c_cacheGlyphCountY = 8;
const uint32_t c_cacheGlyphCount = c_cacheGlyphCountX * c_cacheGlyphCountY;
const uint32_t c_cacheGlyphDimX = c_cacheGlyphSize * c_cacheGlyphCountX;
const uint32_t c_cacheGlyphDimY = c_cacheGlyphSize * c_cacheGlyphCountY;

const render::Handle c_glyphsTargetSetId(L"Spark_GlyphsTargetSet");

const int32_t c_spriteAtlasDim = 2048;		//!< Size of sprite bitmap cache.
const float c_spriteSupersample = 3.0f;		//!< Cells are rendered at this multiple of their on screen size.
const int32_t c_spriteMargin = 1;			//!< Transparent border around each cell.
const int32_t c_spriteMaxCellDim = 1024;	//!< Largest cell we allow, so a single huge sprite can't consume the atlas.
const float c_spriteScaleTolerance = 1.25f;	//!< Screen scale may drift before we re-capture.

//! Cell size, in atlas pixels, needed to hold given bounds at given scale.
void cellSize(const Aabb2& bounds, float scale, int32_t& outWidth, int32_t& outHeight)
{
	outWidth = (int32_t)std::ceil((bounds.mx.x - bounds.mn.x) * scale) + 2 * c_spriteMargin;
	outHeight = (int32_t)std::ceil((bounds.mx.y - bounds.mn.y) * scale) + 2 * c_spriteMargin;
}

//! True if an already allocated cell can hold given bounds at given scale.
bool cellFits(const Packer::Rectangle& rect, const Aabb2& bounds, float scale)
{
	int32_t width, height;
	cellSize(bounds, scale, width, height);
	return width <= rect.width && height <= rect.height;
}

//! Resolution a cell actually afford for given bounds; the axis which fit worst, since
//! that is what limits how much detail survive.
float cellScale(const Packer::Rectangle& rect, const Aabb2& bounds)
{
	const float width = bounds.mx.x - bounds.mn.x;
	const float height = bounds.mx.y - bounds.mn.y;
	if (width <= FUZZY_EPSILON || height <= FUZZY_EPSILON)
		return 0.0f;
	return std::min(
		(rect.width - 2 * c_spriteMargin) / width,
		(rect.height - 2 * c_spriteMargin) / height);
}

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.AccDisplayRenderer", AccDisplayRenderer, IDisplayRenderer)

AccDisplayRenderer::AccDisplayRenderer()
	: m_nextIndex(0)
	, m_frameBounds(0.0f, 0.0f, 0.0f, 0.0f)
	, m_frameTransform(0.0f, 0.0f, 1.0f, 1.0f)
	, m_viewSize(0.0f, 0.0f, 0.0f, 0.0f)
	, m_clearBackground(false)
	, m_maskWrite(false)
	, m_maskIncrement(false)
	, m_maskReference(0)
	, m_glyphFilter(0)
	, m_glyphColor(0.0f, 0.0f, 0.0f, 0.0f)
	, m_glyphFilterColor(0.0f, 0.0f, 0.0f, 0.0f)
	, m_firstFrame(true)
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
	bool clearBackground)
{
	m_resourceManager = resourceManager;
	m_renderSystem = renderSystem;
	m_gradientCache = new AccGradientCache(m_renderSystem);
	m_textureCache = new AccTextureCache(m_resourceManager, m_renderSystem, false);
	m_clearBackground = clearBackground;

	m_shapeResources = new AccShapeResources();
	if (!m_shapeResources->create(resourceManager))
	{
		log::error << L"Unable to create accelerated display renderer; failed to load shape resources." << Endl;
		return false;
	}

	AlignedVector< render::VertexElement > fillVertexElements(5);
	fillVertexElements[0] = render::VertexElement(render::DataUsage::Position, render::DtFloat2, offsetof(AccShape::FillVertex, pos));
	fillVertexElements[1] = render::VertexElement(render::DataUsage::Custom, render::DtByte4N, offsetof(AccShape::FillVertex, curvature), 0);
	fillVertexElements[2] = render::VertexElement(render::DataUsage::Custom, render::DtFloat2, offsetof(AccShape::FillVertex, texCoord), 1);
	fillVertexElements[3] = render::VertexElement(render::DataUsage::Custom, render::DtFloat4, offsetof(AccShape::FillVertex, texRect), 2);
	fillVertexElements[4] = render::VertexElement(render::DataUsage::Color, render::DtByte4N, offsetof(AccShape::FillVertex, color), 0);
	T_FATAL_ASSERT(render::getVertexSize(fillVertexElements) == sizeof(AccShape::FillVertex));

	m_fillVertexPool = new AccShapeVertexPool(renderSystem);
	if (!m_fillVertexPool->create(fillVertexElements))
	{
		log::error << L"Unable to create accelerated display renderer; failed to create vertex pool (fill)." << Endl;
		return false;
	}

	AlignedVector< render::VertexElement > lineVertexElements(3);
	lineVertexElements[0] = render::VertexElement(render::DataUsage::Position, render::DtFloat2, offsetof(AccShape::LineVertex, pos));
	lineVertexElements[1] = render::VertexElement(render::DataUsage::Custom, render::DtInteger1, offsetof(AccShape::LineVertex, lineOffset), 0);
	lineVertexElements[2] = render::VertexElement(render::DataUsage::Custom, render::DtInteger1, offsetof(AccShape::LineVertex, lineCount), 1);
	T_FATAL_ASSERT(render::getVertexSize(lineVertexElements) == sizeof(AccShape::LineVertex));

	m_lineVertexPool = new AccShapeVertexPool(renderSystem);
	if (!m_lineVertexPool->create(lineVertexElements))
	{
		log::error << L"Unable to create accelerated display renderer; failed to create vertex pool (line)." << Endl;
		return false;
	}

	m_glyph = new AccGlyph();
	if (!m_glyph->create(resourceManager, renderSystem))
	{
		log::error << L"Unable to create accelerated display renderer; failed to create glyph list." << Endl;
		return false;
	}

	// Create glyph cache target.
	const render::RenderTargetSetCreateDesc rtscd = {
		.count = 1,
		.width = c_cacheGlyphDimX,
		.height = c_cacheGlyphDimY,
		.targets = {
			{ .format = render::TfR8 } }
	};
	m_glyphTargetSet = renderSystem->createRenderTargetSet(rtscd, nullptr, T_FILE_LINE_W);

	// Create sprite cache atlas; need stencil since captured sprites may contain masks.
	const render::RenderTargetSetCreateDesc rtscds = {
		.count = 1,
		.width = c_spriteAtlasDim,
		.height = c_spriteAtlasDim,
		.createDepthStencil = true,
		.ignoreStencil = false,
		.targets = {
			{ .format = render::TfR8G8B8A8 } }
	};
	m_spriteTargetSet = renderSystem->createRenderTargetSet(rtscds, nullptr, T_FILE_LINE_W);
	if (!m_spriteTargetSet)
	{
		log::error << L"Unable to create accelerated display renderer; failed to create sprite cache atlas." << Endl;
		return false;
	}

	m_spritePacker = new Packer(c_spriteAtlasDim, c_spriteAtlasDim);

	m_quad = new AccQuad();
	if (!m_quad->create(resourceManager, renderSystem))
	{
		log::error << L"Unable to create accelerated display renderer; failed to create quad shape." << Endl;
		return false;
	}

	return true;
}

void AccDisplayRenderer::destroy()
{
	m_renderSystem = nullptr;

	safeDestroy(m_glyph);
	safeDestroy(m_glyphTargetSet);
	safeDestroy(m_spriteTargetSet);
	safeDestroy(m_quad);

	m_spritePacker = nullptr;
	m_compositeCache = nullptr;
	safeDestroy(m_gradientCache);
	safeDestroy(m_textureCache);

	for (auto i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
		safeDestroy(i->second.shape);

	for (auto i = m_glyphCache.begin(); i != m_glyphCache.end(); ++i)
		safeDestroy(i->second.shape);

	m_shapeCache.clear();
	m_glyphCache.clear();

	safeDestroy(m_shapeResources);
	safeDestroy(m_fillVertexPool);
	safeDestroy(m_lineVertexPool);
}

void AccDisplayRenderer::beginSetup(render::RenderGraph* renderGraph)
{
	m_renderGraph = renderGraph;
	m_glyphsTargetSetId = m_renderGraph->addExplicitTargetSet(L"Spark glyphs", m_glyphTargetSet);
	m_spritesTargetSetId = m_renderGraph->addExplicitTargetSet(L"Spark sprites", m_spriteTargetSet);
	m_renderPassOutput = new render::RenderPass(L"Spark");
	m_renderPassOutput->addInput(m_glyphsTargetSetId);
	m_renderPassOutput->addInput(m_spritesTargetSetId);

	if (m_clearBackground)
	{
		const render::Clear cl = {
			.mask = render::CfColor | render::CfDepth | render::CfStencil,
			.colors = { Color4f(1.0f, 1.0f, 1.0f, 0.0) },
			.depth = 1.0f,
			.stencil = 0
		};
		m_renderPassOutput->setOutput(render::RGTargetSet::Output, cl, render::TfNone, render::TfColor | render::TfDepth);
	}
	else
	{
		const render::Clear cl = {
			.mask = render::CfStencil,
			.stencil = 0
		};
		m_renderPassOutput->setOutput(render::RGTargetSet::Output, cl, render::TfColor | render::TfDepth, render::TfColor | render::TfDepth);
	}

	m_renderPassGlyph = new render::RenderPass(L"Spark glyphs");
	if (!m_firstFrame)
		m_renderPassGlyph->setOutput(m_glyphsTargetSetId, render::TfColor, render::TfColor);
	else
	{
		const render::Clear cl = {
			.mask = render::CfColor,
			.colors = { Color4f(0.5f, 0.5f, 0.5f, 1.0) }
		};
		m_renderPassGlyph->setOutput(m_glyphsTargetSetId, cl, render::TfNone, render::TfColor);
	}

	m_renderPassSprite = new render::RenderPass(L"Spark sprites");
	m_renderPassSprite->addInput(m_glyphsTargetSetId);

	const render::Clear cl = {
		.mask = render::CfStencil,
		.stencil = 0
	};
	m_renderPassSprite->setOutput(m_spritesTargetSetId, cl, render::TfColor, render::TfColor);

	m_renderPassTarget = m_renderPassOutput;

	m_frameTransform.set(0.0f, 0.0f, 1.0f, 1.0f);
}

void AccDisplayRenderer::endSetup()
{
	if (!m_renderPassGlyph->getBuilds().empty() || m_firstFrame)
		m_renderGraph->addPass(m_renderPassGlyph);
	if (!m_renderPassSprite->getBuilds().empty())
		m_renderGraph->addPass(m_renderPassSprite);
	if (!m_renderPassOutput->getBuilds().empty())
		m_renderGraph->addPass(m_renderPassOutput);

	m_renderGraph = nullptr;
	m_renderPassOutput = nullptr;
	m_renderPassGlyph = nullptr;
	m_renderPassSprite = nullptr;
	m_renderPassTarget = nullptr;

	m_firstFrame = false;
}

void AccDisplayRenderer::flushCaches()
{
	m_gradientCache->clear();
	m_textureCache->clear();

	for (auto i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
		safeDestroy(i->second.shape);

	for (auto i = m_glyphCache.begin(); i != m_glyphCache.end(); ++i)
		safeDestroy(i->second.shape);

	m_shapeCache.clear();
	m_glyphCache.clear();

	m_nextIndex = 0;

	m_spritePacker->reset();
	m_spriteAtlasFull = false;
	m_compositeSprite = nullptr;
	m_compositeCache = nullptr;
	m_captureSprite = nullptr;
	m_captureTransformInv = Matrix33::identity();
}

void AccDisplayRenderer::setClearBackground(bool clearBackground)
{
	m_clearBackground = clearBackground;
}

// IDisplayRenderer

bool AccDisplayRenderer::wantDirtyRegion() const
{
	return false;
}

void AccDisplayRenderer::begin(
	const Dictionary& dictionary,
	const Color4f& backgroundColor,
	const Aabb2& frameBounds,
	const Vector4& frameTransform,
	float viewWidth,
	float viewHeight,
	const Aabb2& dirtyRegion)
{
	m_frameCount++;

	if (m_spriteAtlasFull)
	{
		m_spritePacker->reset();
		m_spriteAtlasFull = false;
	}

	m_frameBounds.set(frameBounds.mn.x, frameBounds.mn.y, frameBounds.mx.x, frameBounds.mx.y);
	m_frameTransform = frameTransform;
	m_viewSize.set(viewWidth, viewHeight, 1.0f / viewWidth, 1.0f / viewHeight);

	const Vector2 Ft_offset(frameTransform.x(), frameTransform.y());
	const Vector2 Ft_scale(frameTransform.z(), frameTransform.w());
	m_frameBoundsVisible.mn = frameBounds.mn + (frameBounds.mx - frameBounds.mn) * (Ft_offset / Ft_scale);
	m_frameBoundsVisible.mx = frameBounds.mn + (frameBounds.mx - frameBounds.mn) * ((Vector2::one() - Ft_offset) / Ft_scale);

	m_glyph->beginFrame();

	m_maskWrite = false;
	m_maskIncrement = false;
	m_maskReference = 0;

	// Drawing start out targeting the output pass; a sprite capture redirect it.
	m_renderPassTarget = m_renderPassOutput;
	m_captureSprite = nullptr;
	m_captureTransformInv = Matrix33::identity();
	m_compositeSprite = nullptr;
	m_compositeCache = nullptr;

	// Dirty regions not used but set region to entire frame so we can
	// cull shapes trivially later.
	m_dirtyRegion = m_frameBoundsVisible;
}

bool AccDisplayRenderer::beginSprite(const SpriteInstance& sprite, const Matrix33& transform, ColorTransform& inoutCxform)
{
	// Check if caching is allowed/suitable for this sprite.
	if (
		!sprite.getCacheAsBitmap() ||
		m_captureSprite != nullptr ||
		m_maskWrite ||
		sprite.getBlendMode() != SbmDefault ||
		sprite.getMask() != nullptr)
		return true;

	const float scale = spriteAtlasScale(transform);
	if (scale <= 0.0f)
		return true;

	Ref< AccCachedSprite > cached = static_cast< AccCachedSprite* >(sprite.getCacheObject());
	if (cached != nullptr && (cached->packer != m_spritePacker || cached->epoch != m_spritePacker->getEpoch()))
		cached = nullptr;

	if (
		cached != nullptr &&
		cached->valid &&
		cached->version == sprite.getCacheVersion() &&
		scale <= cached->scale * c_spriteScaleTolerance &&
		scale >= cached->scale / c_spriteScaleTolerance)
	{
		m_compositeSprite = &sprite;
		m_compositeCache = cached;
		m_compositeTransform = transform;
		m_compositeCxform = inoutCxform;
		return false;
	}

	const Aabb2 bounds = sprite.getVisibleLocalBounds();
	if (bounds.empty())
		return true;

	// Re-capture into the same cell if possible.
	Ref< AccCachedSprite > allocated = cached;
	if (allocated != nullptr && !cellFits(allocated->rect, bounds, scale))
		allocated = nullptr;

	if (allocated == nullptr)
	{
		allocated = allocateCachedSprite(bounds, scale);
		if (!allocated)
			return true;
		sprite.setCacheObject(allocated);
	}
	else
	{
		allocated->bounds = bounds;
		allocated->scale = cellScale(allocated->rect, bounds);
	}

	allocated->version = sprite.getCacheVersion();
	allocated->valid = false;

	// Queued glyphs belong to the enclosing pass; flush before we redirect drawing.
	renderEnqueuedGlyphs();

	m_compositeSprite = &sprite;
	m_compositeCache = allocated;
	m_compositeTransform = transform;
	m_compositeCxform = inoutCxform;

	m_captureSprite = &sprite;
	m_captureTransformInv = transform.inverse();
	inoutCxform = c_cxfIdentity;

	m_savedFrameBounds = m_frameBounds;
	m_savedFrameTransform = m_frameTransform;
	m_savedMaskReference = m_maskReference;
	m_renderPassTarget = m_renderPassSprite;
	m_maskReference = 0;

	const float invDim = 1.0f / c_spriteAtlasDim;
	const Vector4 localBounds(bounds.mn.x, bounds.mn.y, bounds.mx.x, bounds.mx.y);

	// Clear cell.
	m_quad->render(
		m_renderPassSprite,
		bounds,
		Matrix33::identity(),
		localBounds,
		Vector4(
			allocated->rect.x * invDim,
			allocated->rect.y * invDim,
			allocated->rect.width * invDim,
			allocated->rect.height * invDim),
		c_cxfZero,
		nullptr,
		Vector4::zero(),
		false,
		false,
		0);

	// Map the sprite's local bounds onto the cell, inside the margin.
	m_frameBounds = localBounds;
	m_frameTransform.set(
		(allocated->rect.x + c_spriteMargin) * invDim,
		(allocated->rect.y + c_spriteMargin) * invDim,
		(allocated->rect.width - 2 * c_spriteMargin) * invDim,
		(allocated->rect.height - 2 * c_spriteMargin) * invDim);

	return true;
}

void AccDisplayRenderer::endSprite(const SpriteInstance& sprite, const Matrix33& transform)
{
	if (&sprite == m_captureSprite)
	{
		// Flush glyphs drawn into the cell before restoring state.
		renderEnqueuedGlyphs();

		m_frameBounds = m_savedFrameBounds;
		m_frameTransform = m_savedFrameTransform;
		m_maskReference = m_savedMaskReference;
		m_renderPassTarget = m_renderPassOutput;

		m_captureSprite = nullptr;
		m_captureTransformInv = Matrix33::identity();

		m_compositeCache->valid = true;
		compositeCachedSprite();
	}
	else if (&sprite == m_compositeSprite)
		compositeCachedSprite();
}

float AccDisplayRenderer::spriteAtlasScale(const Matrix33& transform) const
{
	// Visible part of the frame maps onto the entire view, giving pixels per twip.
	const Vector2 frameSize = m_frameBoundsVisible.mx - m_frameBoundsVisible.mn;
	if (frameSize.x <= FUZZY_EPSILON || frameSize.y <= FUZZY_EPSILON)
		return 0.0f;

	const float px = m_viewSize.x() / frameSize.x;
	const float py = m_viewSize.y() / frameSize.y;

	// Length of the transform's basis vectors; take the larger so a non-uniformly
	// scaled sprite is never under-sampled along its major axis.
	const float sx = Vector2(transform.e11, transform.e21).length() * px;
	const float sy = Vector2(transform.e12, transform.e22).length() * py;

	const float scale = std::max(sx, sy) * c_spriteSupersample;
	return scale > FUZZY_EPSILON ? scale : 0.0f;
}

Ref< AccCachedSprite > AccDisplayRenderer::allocateCachedSprite(const Aabb2& bounds, float scale)
{
	int32_t width, height;
	cellSize(bounds, scale, width, height);

	if (width <= 2 * c_spriteMargin || height <= 2 * c_spriteMargin)
		return nullptr;
	if (width > c_spriteMaxCellDim || height > c_spriteMaxCellDim)
		return nullptr;

	Packer::Rectangle rect;
	if (!m_spritePacker->insert(width, height, rect))
	{
		m_spriteAtlasFull = true;
		return nullptr;
	}

	Ref< AccCachedSprite > cached = new AccCachedSprite();
	cached->packer = m_spritePacker;
	cached->epoch = m_spritePacker->getEpoch();
	cached->rect = rect;
	cached->bounds = bounds;
	cached->scale = scale;
	cached->valid = false;
	return cached;
}

void AccDisplayRenderer::compositeCachedSprite()
{
	T_ASSERT(m_compositeCache);

	// Ordered relative to queued glyphs just like any other shape.
	renderEnqueuedGlyphs();

	const float invDim = 1.0f / c_spriteAtlasDim;
	const Packer::Rectangle& rect = m_compositeCache->rect;

	m_quad->renderCached(
		m_renderPassOutput,
		m_compositeCache->bounds,
		m_compositeTransform,
		m_frameBounds,
		m_frameTransform,
		m_compositeCxform,
		m_spriteTargetSet->getColorTexture(0),
		Vector4(
			(rect.x + c_spriteMargin) * invDim,
			(rect.y + c_spriteMargin) * invDim,
			(rect.width - 2 * c_spriteMargin) * invDim,
			(rect.height - 2 * c_spriteMargin) * invDim),
		m_maskReference);

	m_compositeSprite = nullptr;
	m_compositeCache = nullptr;
}

void AccDisplayRenderer::beginEdit(const EditInstance& edit, const Matrix33& transform)
{
	if (edit.getRenderClipMask())
	{
		beginMask(true);
		renderQuad(transform, edit.getTextBounds(), c_cxfWhite);
		endMask();
	}
}

void AccDisplayRenderer::endEdit(const EditInstance& edit, const Matrix33& transform)
{
	if (edit.getRenderClipMask())
	{
		beginMask(false);
		renderQuad(transform, edit.getTextBounds(), c_cxfWhite);
		endMask();
	}
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
		T_ASSERT(m_maskReference < 255);
		m_maskReference++;
	}
	else
	{
		T_ASSERT(m_maskReference > 0);
		m_maskReference--;
	}
}

void AccDisplayRenderer::renderShape(const Dictionary& dictionary, const Matrix33& transform, const Aabb2& clipBounds, const Shape& shape, const ColorTransform& cxform, uint8_t blendMode)
{
	Ref< AccShape > accShape;

	// Check if shape is within frame bounds.
	// if (!rectangleVisible(m_dirtyRegion, transform * shape.getShapeBounds()))
	//	return;

	// Get accelerated shape.
	const int32_t tag = shape.getCacheTag();
	auto it = m_shapeCache.find(tag);
	if (it == m_shapeCache.end())
	{
		accShape = new AccShape(m_renderSystem, m_shapeResources, m_fillVertexPool, m_lineVertexPool);
		if (!accShape->createFromShape(
				m_gradientCache,
				m_textureCache,
				dictionary,
				shape))
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

	accShape->render(
		m_renderPassTarget,
		m_captureTransformInv * transform,
		Vector4(clipBounds.mn.x, clipBounds.mn.y, clipBounds.mx.x, clipBounds.mx.y),
		m_frameBounds,
		m_frameTransform,
		cxform,
		m_maskWrite,
		m_maskIncrement,
		m_maskReference,
		blendMode);
}

void AccDisplayRenderer::renderMorphShape(const Dictionary& dictionary, const Matrix33& transform, const Aabb2& clipBounds, const MorphShape& shape, const ColorTransform& cxform)
{
}

void AccDisplayRenderer::renderGlyph(
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
	const Color4f& filterColor)
{
	// Only support embedded fonts.
	if (!glyph)
		return;

	const float coordScale = font->getCoordinateType() == Font::CtTwips ? 1.0f / 1000.0f : 1.0f / (20.0f * 1000.0f);
	const float fontScale = coordScale * fontHeight;
	const Matrix33 glyphTransform = transform * scale(fontScale, fontScale);
	const Color4f glyphColor = color * cxform.mul + cxform.add;

	// Check if shape is within frame bounds; see renderQuad regarding capturing.
	if (m_captureSprite == nullptr && !rectangleVisible(m_dirtyRegion, glyphTransform * glyph->getShapeBounds()))
		return;

	if (m_glyphFilter != filter || !colorsEqual(glyphColor, m_glyphColor))
	{
		renderEnqueuedGlyphs();
		m_glyphFilter = filter;
		m_glyphColor = glyphColor;
		m_glyphFilterColor = filterColor;
	}

	const uint32_t tag = glyph->getCacheTag();

	auto it1 = m_glyphCache.find(tag);
	if (it1 == m_glyphCache.end())
	{
		Ref< AccShape > accShape = new AccShape(m_renderSystem, m_shapeResources, m_fillVertexPool, m_lineVertexPool);
		if (!accShape->createFromGlyph(
				m_gradientCache,
				m_textureCache,
				dictionary,
				*glyph))
			return;

		m_glyphCache[tag].shape = accShape;
		m_glyphCache[tag].index = -1;

		it1 = m_glyphCache.find(tag);
		T_ASSERT(it1 != m_glyphCache.end());
	}

	AccShape* accShape = it1->second.shape;
	T_ASSERT(accShape);

	const float cachePixelDx = 1.0f / c_cacheGlyphDimX;
	const float cachePixelDy = 1.0f / c_cacheGlyphDimY;

	const Aabb2& bounds = accShape->getBounds();

	// Get cached glyph target.
	if (it1->second.index < 0)
	{
		// Glyph not cached; pick a cell by cycling, which means the oldest glyph get
		// discarded not referenced in this frame.
		int32_t index = -1;
		for (uint32_t i = 0; i < c_cacheGlyphCount; ++i)
		{
			const int32_t candidate = m_nextIndex;
			if (++m_nextIndex >= (int32_t)c_cacheGlyphCount)
				m_nextIndex = 0;

			bool referencedThisFrame = false;
			for (const auto& cache : m_glyphCache)
			{
				if (cache.second.index == candidate && cache.second.lastFrame == m_frameCount)
				{
					referencedThisFrame = true;
					break;
				}
			}

			if (!referencedThisFrame)
			{
				index = candidate;
				break;
			}
		}

		if (index < 0)
			return;

		for (auto& cache : m_glyphCache)
			if (cache.second.index == index)
				cache.second.index = -1;

		const int32_t column = index & (c_cacheGlyphCountX - 1);
		const int32_t row = index / c_cacheGlyphCountX;

		const Vector4 frameSize(bounds.mn.x, bounds.mn.y, bounds.mx.x, bounds.mx.y);
		const Vector4 viewOffset(
			float(column) / c_cacheGlyphCountX,
			float(row) / c_cacheGlyphCountY,
			1.0f / c_cacheGlyphCountX,
			1.0f / c_cacheGlyphCountY);
		const Vector4 viewOffsetWithMargin = viewOffset + Vector4(cachePixelDx * c_cacheGlyphMargin, cachePixelDy * c_cacheGlyphMargin, -cachePixelDx * c_cacheGlyphMargin * 2.0f, -cachePixelDy * c_cacheGlyphMargin * 2.0f);

		// Clear previous glyph by drawing a solid quad at it's place.
		m_quad->render(
			m_renderPassGlyph,
			bounds,
			Matrix33::identity(),
			frameSize,
			viewOffset,
			c_cxfZero,
			0,
			Vector4::zero(),
			false,
			false,
			0);

		accShape->render(
			m_renderPassGlyph,
			Matrix33::identity(),
			Vector4(bounds.mn.x, bounds.mn.y, bounds.mx.x, bounds.mx.y),
			frameSize,
			viewOffsetWithMargin,
			c_cxfYellow,
			false,
			false,
			false,
			SbmDefault);

		it1->second.index = index;
	}

	// Stamp the cell so nothing else claim it before our draw has sampled it.
	it1->second.lastFrame = m_frameCount;

	const int32_t column = it1->second.index & (c_cacheGlyphCountX - 1);
	const int32_t row = it1->second.index / c_cacheGlyphCountX;

	m_glyph->add(
		bounds,
		m_captureTransformInv * glyphTransform,
		Vector4(
			float(column) / c_cacheGlyphCountX,
			float(row) / c_cacheGlyphCountY,
			1.0f / c_cacheGlyphCountX,
			1.0f / c_cacheGlyphCountY));
}

void AccDisplayRenderer::renderQuad(const Matrix33& transform, const Aabb2& bounds, const ColorTransform& cxform)
{
	if (m_captureSprite == nullptr && !rectangleVisible(m_dirtyRegion, transform * bounds))
		return;

	m_quad->render(
		m_renderPassTarget,
		bounds,
		m_captureTransformInv * transform,
		m_frameBounds,
		m_frameTransform,
		cxform,
		0,
		Vector4::zero(),
		m_maskWrite,
		m_maskIncrement,
		m_maskReference);
}

void AccDisplayRenderer::renderCanvas(const Matrix33& transform, const Canvas& canvas, const ColorTransform& cxform, uint8_t blendMode)
{
	Ref< AccShape > accShape;

	const int32_t tag = canvas.getCacheTag();
	SmallMap< int32_t, ShapeCache >::iterator it = m_shapeCache.find(tag);
	if (it == m_shapeCache.end() || it->second.tag != canvas.getDirtyTag())
	{
		if (it != m_shapeCache.end() && it->second.shape)
		{
			it->second.shape->destroy();
			it->second.shape = nullptr;
		}

		accShape = new AccShape(m_renderSystem, m_shapeResources, m_fillVertexPool, m_lineVertexPool);
		if (!accShape->createFromCanvas(
				m_gradientCache,
				m_textureCache,
				canvas))
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

	if (m_captureSprite == nullptr && !rectangleVisible(m_dirtyRegion, transform * accShape->getBounds()))
		return;

	renderEnqueuedGlyphs();

	accShape->render(
		m_renderPassTarget,
		m_captureTransformInv * transform,
		m_frameBounds,
		m_frameBounds,
		m_frameTransform,
		cxform,
		m_maskWrite,
		m_maskIncrement,
		m_maskReference,
		blendMode);
}

void AccDisplayRenderer::end()
{
	renderEnqueuedGlyphs();

	m_glyph->endFrame();

	// Don't flush cache if it doesn't contain that many shapes.
	if (m_shapeCache.size() >= c_maxCacheSize)
	{
		// Nuke cached shapes which hasn't been used for X number of frames.
		for (auto i = m_shapeCache.begin(); i != m_shapeCache.end();)
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
		for (auto i = m_shapeCache.begin(); i != m_shapeCache.end(); ++i)
			i->second.unusedCount++;
	}

	m_gradientCache->synchronize();
}

void AccDisplayRenderer::renderEnqueuedGlyphs()
{
	m_glyph->render(
		m_renderPassTarget,
		m_glyphsTargetSetId,
		m_frameBounds,
		m_frameTransform,
		m_maskReference,
		m_glyphFilter,
		m_glyphColor,
		m_glyphFilterColor);
}

}
