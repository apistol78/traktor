/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/Packer.h"
#include "Flash/Sprite.h"
#include "Flash/SpriteInstance.h"
#include "Flash/SwfTypes.h"
#include "Flash/Acc/AccQuad.h"
#include "Flash/Acc/AccShape.h"
#include "Flash/Acc/AccShapeRenderer.h"
#include "Render/IRenderSystem.h"
#include "Render/RenderTargetSet.h"
#include "Render/Context/RenderContext.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

#if defined(__ANDROID__) || defined(__IOS__)
const uint32_t c_cacheWidth = 1024;
const uint32_t c_cacheHeight = 1024;
#else
const uint32_t c_cacheWidth = 2048;
const uint32_t c_cacheHeight = 2048;
#endif
const uint32_t c_cacheMargin = 1;

const ColorTransform c_cxfZero(Color4f(0.0f, 0.0f, 0.0f, 0.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));
const ColorTransform c_cxfIdentity(Color4f(1.0f, 1.0f, 1.0f, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f));

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccShapeRenderer", AccShapeRenderer, Object)

AccShapeRenderer::AccShapeRenderer()
:	m_packer(0)
,	m_renderIntoSlot(0)
,	m_renderFromSlot(0)
,	m_cacheAsBitmap(0)
{
}

bool AccShapeRenderer::create(render::IRenderSystem* renderSystem, resource::IResourceManager* resourceManager)
{
	render::RenderTargetSetCreateDesc rtscd;
	rtscd.count = 1;
	rtscd.width = c_cacheWidth;
	rtscd.height = c_cacheHeight;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = true;
	rtscd.usingPrimaryDepthStencil = false;
	rtscd.ignoreStencil = false;
	rtscd.targets[0].format = render::TfR8G8B8A8;

	m_renderTargetShapes = renderSystem->createRenderTargetSet(rtscd);
	if (!m_renderTargetShapes)
	{
		log::error << L"Unable to create accelerated display renderer; failed to create shape cache target" << Endl;
		return false;
	}

	m_quad = new AccQuad();
	if (!m_quad->create(resourceManager, renderSystem))
	{
		log::error << L"Unable to create accelerated display renderer; failed to create quad shape" << Endl;
		return false;
	}

	m_packer = new Packer(c_cacheWidth, c_cacheHeight);
	return true;
}

void AccShapeRenderer::destroy()
{
	safeDestroy(m_quad);
	safeDestroy(m_renderTargetShapes);
}

void AccShapeRenderer::beginFrame()
{
	for (int32_t i = 0; i < m_cache.size(); ++i)
	{
		if (++m_cache[i].unused >= 20)
		{
			m_cache.resize(0);
			m_packer = new Packer(c_cacheWidth, c_cacheHeight);
			break;
		}
	}

	m_renderIntoSlot = -1;
	m_renderFromSlot = -1;
}

void AccShapeRenderer::endFrame()
{
}

void AccShapeRenderer::beginSprite(
	render::RenderContext* renderContext,
	const SpriteInstance& sprite,
	const Vector4& frameBounds,
	const Vector4& frameTransform,
	const Vector4& viewSize,
	const Matrix33& transform,
	uint8_t maskReference
)
{
	if (sprite.getCacheAsBitmap())
	{
		if (m_cacheAsBitmap++ == 0)
		{
			beginCacheAsBitmap(
				renderContext,
				sprite,
				frameBounds,
				frameTransform,
				viewSize,
				transform,
				maskReference
			);
		}
	}
}

void AccShapeRenderer::endSprite(
	render::RenderContext* renderContext,
	const SpriteInstance& sprite,
	const Vector4& frameBounds,
	const Vector4& frameTransform,
	const Matrix33& transform,
	uint8_t maskReference
)
{
	if (sprite.getCacheAsBitmap())
	{
		if (--m_cacheAsBitmap == 0)
		{
			endCacheAsBitmap(
				renderContext,
				frameBounds,
				frameTransform,
				transform,
				maskReference
			);
		}
	}
}

void AccShapeRenderer::render(
	render::RenderContext* renderContext,
	AccShape* shape,
	int32_t tag,
	const ColorTransform& cxform,
	const Vector4& frameBounds,
	const Vector4& frameTransform,
	const Matrix33& transform,
	bool maskWrite,
	bool maskIncrement,
	uint8_t maskReference,
	uint8_t blendMode
)
{
	if (m_renderIntoSlot >= 0)
	{
		const Cache& c = m_cache[m_renderIntoSlot];

		Vector4 cacheFrameSize(c.bounds.mn.x, c.bounds.mn.y, c.bounds.mx.x, c.bounds.mx.y);
		Vector4 cacheViewOffset(
			float(c.x) / c_cacheWidth,
			float(c.y) / c_cacheHeight,
			float(c.width) / c_cacheWidth,
			float(c.height) / c_cacheHeight
		);

		Matrix33 delta = c.transform.inverse() * transform;
		shape->render(
			renderContext,
			delta,
			cacheFrameSize,
			cacheViewOffset,
			cxform,
			maskWrite,
			maskIncrement,
			maskReference,
			blendMode
		);
	}
	else if (m_renderFromSlot < 0)
	{
		shape->render(
			renderContext,
			transform,
			frameBounds,
			frameTransform,
			cxform,
			maskWrite,
			maskIncrement,
			maskReference,
			blendMode
		);
	}
}

bool AccShapeRenderer::shouldCull() const
{
	return m_renderIntoSlot < 0;
}

void AccShapeRenderer::beginCacheAsBitmap(
	render::RenderContext* renderContext,
	const SpriteInstance& spriteInstance,
	const Vector4& frameBounds,
	const Vector4& frameTransform,
	const Vector4& viewSize,
	const Matrix33& transform,
	uint8_t maskReference
)
{
	T_FATAL_ASSERT (m_renderIntoSlot < 0);
	T_FATAL_ASSERT (m_renderFromSlot < 0);

	int32_t tag = spriteInstance.getSprite()->getCacheTag();
	Aabb2 bounds = spriteInstance.getLocalBounds();

	// Extract axis scale factors.
	float scaleX = Vector2(transform.e11, transform.e12).length();
	float scaleY = Vector2(transform.e21, transform.e22).length();

	// Calculate size of shape's bounds on stage.
	float stageW = (bounds.mx.x - bounds.mn.x) * scaleX;
	float stageH = (bounds.mx.y - bounds.mn.y) * scaleY;

	// Normalize size of shape.
	float tx = stageW / (frameBounds.z() - frameBounds.x());
	float ty = stageH / (frameBounds.w() - frameBounds.y());

	// Expand by current view size and offsets.
	float sx = tx * frameTransform.z() * viewSize.x();
	float sy = ty * frameTransform.w() * viewSize.y();

	int32_t pixelWidth = int32_t(sx + 0.5f);
	int32_t pixelHeight = int32_t(sy + 0.5f);

	// Check if sprite instance already cached.
	int32_t slot = -1;
	for (int32_t i = 0; i < m_cache.size(); ++i)
	{
		if (m_cache[i].tag == tag && m_cache[i].width == pixelWidth && m_cache[i].height == pixelHeight)
		{
			slot = i;
			break;
		}
	}

	// If not cached then allocate a new region and begin rendering into cache.
	if (slot < 0)
	{
		Packer::Rectangle r;
		if (m_packer->insert(pixelWidth + c_cacheMargin * 2, pixelHeight + c_cacheMargin * 2, r))
		{
			Cache c;
			c.tag = tag;
			c.x = r.x + c_cacheMargin;
			c.y = r.y + c_cacheMargin;
			c.width = r.width - c_cacheMargin * 2;
			c.height = r.height - c_cacheMargin * 2;
			c.unused = 0;
			c.bounds = bounds;
			c.transform = transform;

			slot = int32_t(m_cache.size());
			m_cache.push_back(c);

			render::TargetBeginRenderBlock* renderBlockBegin = renderContext->alloc< render::TargetBeginRenderBlock >("Flash sprite cache begin");
			renderBlockBegin->renderTargetSet = m_renderTargetShapes;
			renderBlockBegin->renderTargetIndex = 0;
			renderContext->draw(render::RpOverlay, renderBlockBegin);

			if (slot == 0)
			{
				render::TargetClearRenderBlock* renderBlockClear = renderContext->alloc< render::TargetClearRenderBlock >("Flash sprite cache clear (color|stencil)");
				renderBlockClear->clearMask = render::CfColor | render::CfStencil;
				renderBlockClear->clearColor = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
				renderBlockClear->clearStencil = maskReference;
				renderContext->draw(render::RpOverlay, renderBlockClear);
			}
			else
			{
				render::TargetClearRenderBlock* renderBlockClear = renderContext->alloc< render::TargetClearRenderBlock >("Flash sprite cache clear (stencil)");
				renderBlockClear->clearMask = render::CfStencil;
				renderBlockClear->clearStencil = maskReference;
				renderContext->draw(render::RpOverlay, renderBlockClear);
			}

			m_renderIntoSlot = slot;
		}
	}

	if (slot >= 0)
		m_renderFromSlot = slot;
}

void AccShapeRenderer::endCacheAsBitmap(
	render::RenderContext* renderContext,
	const Vector4& frameBounds,
	const Vector4& frameTransform,
	const Matrix33& transform,
	uint8_t maskReference
)
{
	if (m_renderIntoSlot >= 0)
	{
		render::TargetEndRenderBlock* renderBlockEnd = renderContext->alloc< render::TargetEndRenderBlock >("Flash shape render end");
		renderContext->draw(render::RpOverlay, renderBlockEnd);
		m_renderIntoSlot = -1;
	}

	if (m_renderFromSlot >= 0)
	{
		Cache& c = m_cache[m_renderFromSlot];

		Vector4 textureOffset(
			float(c.x) / c_cacheWidth,
			float(c.y) / c_cacheHeight,
			float(c.width) / c_cacheWidth,
			float(c.height) / c_cacheHeight
		);

		m_quad->render(
			renderContext,
			c.bounds,
			transform,
			frameBounds,
			frameTransform,
			c_cxfIdentity,
			m_renderTargetShapes->getColorTexture(0),
			textureOffset,
			false,
			false,
			maskReference
		);

		c.unused = 0;

		m_renderFromSlot = -1;
	}
}

	}
}
