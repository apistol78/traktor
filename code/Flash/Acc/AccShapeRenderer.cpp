#include <GuillotineBinPack.h>
#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
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

const Matrix33 c_flipped(
	0.0f, 1, 0,
	1, 0, 0,
	0, 0, 1
);

const uint32_t c_cacheWidth = 2048;
const uint32_t c_cacheHeight = 2048;
const uint32_t c_cacheMargin = 1;

const SwfCxTransform c_cxfZero = { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };
const SwfCxTransform c_cxfIdentity = { { 1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f } };

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AccShapeRenderer", AccShapeRenderer, Object)

bool AccShapeRenderer::create(render::IRenderSystem* renderSystem, resource::IResourceManager* resourceManager)
{
	render::RenderTargetSetCreateDesc rtscd;
	rtscd.count = 1;
	rtscd.width = c_cacheWidth;
	rtscd.height = c_cacheHeight;
	rtscd.multiSample = 4;
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

	m_packer.reset(new rbp::GuillotineBinPack(c_cacheWidth, c_cacheHeight));
	return true;
}

void AccShapeRenderer::destroy()
{
	m_packer.release();
	safeDestroy(m_quad);
	safeDestroy(m_renderTargetShapes);
}

void AccShapeRenderer::beginFrame()
{
	for (int32_t i = 0; i < m_cache.size(); ++i)
	{
		if (++m_cache[i].unused >= 2)
		{
			m_cache.resize(0);
			m_packer.reset(new rbp::GuillotineBinPack(c_cacheWidth, c_cacheHeight));
			break;
		}
	}

	m_renderIntoSlot = -1;
	m_renderFromSlot = -1;
}

void AccShapeRenderer::endFrame()
{
	//log::info << L"quad = " << m_quadCount << L", shape = " << m_shapeCount << Endl;
}

void AccShapeRenderer::beginCacheAsBitmap(
	render::RenderContext* renderContext,
	const FlashSpriteInstance& spriteInstance,
	const Vector4& frameSize,
	const Vector4& viewSize,
	const Matrix33& transform
)
{
	T_FATAL_ASSERT (m_renderIntoSlot < 0);

	int32_t tag = spriteInstance.getSprite()->getCacheTag();
	Aabb2 bounds = spriteInstance.getLocalBounds();

	float frameWidth = (frameSize.z() - frameSize.x()) / 20.0f;
	float frameHeight = (frameSize.w() - frameSize.y()) / 20.0f;

	float scaleX = Vector2(transform.e11, transform.e12).length() * (viewSize.x() / frameWidth);
	float scaleY = Vector2(transform.e21, transform.e22).length() * (viewSize.y() / frameHeight);

	int32_t pixelWidth = int32_t(scaleX * (bounds.mx.x - bounds.mn.x) / 20.0f);
	int32_t pixelHeight = int32_t(scaleY * (bounds.mx.y - bounds.mn.y) / 20.0f);

	// Check if sprite instance already cached.
	int32_t slot = -1;
	for (int32_t i = 0; i < m_cache.size(); ++i)
	{
		if (m_cache[i].tag == tag)
		{
			if (
				(!m_cache[i].flipped && m_cache[i].width == pixelWidth && m_cache[i].height == pixelHeight) ||
				( m_cache[i].flipped && m_cache[i].width == pixelHeight && m_cache[i].height == pixelWidth)
			)
			{
				slot = i;
				break;
			}
		}
	}

	// If not cached then allocate a new region and begin rendering into cache.
	if (slot < 0)
	{
		int32_t allocWidth = pixelWidth + c_cacheMargin * 2;
		int32_t allocHeight = pixelHeight + c_cacheMargin * 2;
		rbp::Rect node = m_packer->Insert(
			allocWidth,
			allocHeight,
			false,
			rbp::GuillotineBinPack::RectBestAreaFit,
			rbp::GuillotineBinPack::SplitShorterLeftoverAxis
		);
		if (node.width > 0 && node.height > 0)
		{
			Cache c;
			c.tag = tag;
			c.x = node.x + c_cacheMargin;
			c.y = node.y + c_cacheMargin;
			c.width = node.width - c_cacheMargin * 2;
			c.height = node.height - c_cacheMargin * 2;
			c.flipped = bool(node.width != allocWidth);
			c.unused = 0;
			c.bounds = bounds;
			c.transform = transform;

			slot = m_cache.size();
			m_cache.push_back(c);

			render::TargetBeginRenderBlock* renderBlockBegin = renderContext->alloc< render::TargetBeginRenderBlock >("Flash sprite cache begin");
			renderBlockBegin->renderTargetSet = m_renderTargetShapes;
			renderBlockBegin->renderTargetIndex = 0;
			renderContext->draw(render::RpOverlay, renderBlockBegin);

			if (slot == 0)
			{
				render::TargetClearRenderBlock* renderBlockClear = renderContext->alloc< render::TargetClearRenderBlock >("Flash sprite cache clear");
				renderBlockClear->clearMask = render::CfColor | render::CfStencil;
				renderBlockClear->clearColor = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
				renderContext->draw(render::RpOverlay, renderBlockClear);
			}

			Vector4 cacheFrameSize(bounds.mn.x, bounds.mn.y, bounds.mx.x, bounds.mx.y);
			Vector4 cacheViewOffset(
				float(c.x) / c_cacheWidth,
				float(c.y) / c_cacheHeight,
				float(c.width) / c_cacheWidth,
				float(c.height) / c_cacheHeight
			);

			m_renderIntoSlot = slot;
		}
	}

	if (slot >= 0)
		m_renderFromSlot = slot;
}

void AccShapeRenderer::endCacheAsBitmap(
	render::RenderContext* renderContext,
	const Vector4& frameSize,
	const Vector4& viewOffset,
	const Matrix33& transform
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
			c.flipped ? Aabb2(c.bounds.mn.shuffle< 1, 0 >(), c.bounds.mx.shuffle< 1, 0 >()) : c.bounds,
			transform * (c.flipped ? c_flipped : Matrix33::identity()),
			frameSize,
			viewOffset,
			c_cxfIdentity,
			m_renderTargetShapes->getColorTexture(0),
			textureOffset,
			false,
			false,
			0
		);

		c.unused = 0;

		m_renderFromSlot = -1;
	}
}

void AccShapeRenderer::render(
	render::RenderContext* renderContext,
	AccShape* shape,
	int32_t tag,
	const SwfCxTransform& cxform,
	const Vector4& frameSize,
	const Vector4& viewSize,
	const Vector4& viewOffset,
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
			delta * (c.flipped ? c_flipped : Matrix33::identity()),
			c.flipped ? cacheFrameSize.shuffle< 1, 0, 3, 2 >() : cacheFrameSize,
			cacheViewOffset,
			0.0f,
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
			frameSize,
			viewOffset,
			1.0f,
			cxform,
			maskWrite,
			maskIncrement,
			maskReference,
			blendMode
		);
	}
}

	}
}
