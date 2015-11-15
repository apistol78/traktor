#include "Core/Log/Log.h"
#include "Core/Math/Format.h"
#include "Core/Misc/SafeDestroy.h"
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

const uint32_t c_cacheSlots = 16;
const uint32_t c_cacheSlotWidth = 256;
const uint32_t c_cacheSlotHeight = 256;
const uint32_t c_cacheWidth = c_cacheSlots * c_cacheSlotWidth;
const uint32_t c_cacheHeight = c_cacheSlotHeight;

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
	rtscd.createDepthStencil = false;
	rtscd.usingPrimaryDepthStencil = false;
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

	m_cache.resize(c_cacheSlots);
	for (uint32_t i = 0; i < c_cacheSlots; ++i)
		m_cache[i].tag = 0;

	return true;
}

void AccShapeRenderer::destroy()
{
	safeDestroy(m_quad);
	safeDestroy(m_renderTargetShapes);
}

void AccShapeRenderer::beginFrame()
{
	for (int32_t i = 0; i < c_cacheSlots; ++i)
	{
		if (m_cache[i].tag != 0 && ++m_cache[i].unused >= 2)
			m_cache[i].tag = 0;
	}
}

void AccShapeRenderer::endFrame()
{
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
	// Only permit caching of default blended shapes and not while updating stencil mask.
	if (
		shape->getRenderBatches().size() > 1 &&
		blendMode == SbmDefault &&
		maskWrite == false
	)
	{
		Aabb2 bounds = shape->getBounds();

		float frameWidth = (frameSize.z() - frameSize.x()) / 20.0f;
		float frameHeight = (frameSize.w() - frameSize.y()) / 20.0f;

		float scaleX = Vector2(transform.e11, transform.e12).length() * (viewSize.x() / frameWidth);
		float scaleY = Vector2(transform.e21, transform.e22).length() * (viewSize.y() / frameHeight);

		int32_t pixelWidth = int32_t(scaleX * (bounds.mx.x - bounds.mn.x) / 20.0f);
		int32_t pixelHeight = int32_t(scaleY * (bounds.mx.y - bounds.mn.y) / 20.0f);

		int32_t slot = -1;
		for (int32_t i = 0; i < c_cacheSlots; ++i)
		{
			if (m_cache[i].tag == tag && m_cache[i].width == pixelWidth && m_cache[i].height == pixelHeight)
			{
				slot = i;
				break;
			}
		}

		if (
			slot < 0 &&
			pixelWidth <= c_cacheSlotWidth &&
			pixelHeight <= c_cacheSlotHeight
		)
		{
			for (int32_t i = 0; i < c_cacheSlots; ++i)
			{
				if (m_cache[i].tag == 0)
				{
					slot = i;
					break;
				}
			}
			if (slot >= 0)
			{
				Cache& c = m_cache[slot];
				c.tag = tag;
				c.width = pixelWidth;
				c.height = pixelHeight;

				// Cache shape into offscreen target.
				render::TargetBeginRenderBlock* renderBlockBegin = renderContext->alloc< render::TargetBeginRenderBlock >("Flash shape render begin");
				renderBlockBegin->renderTargetSet = m_renderTargetShapes;
				renderBlockBegin->renderTargetIndex = 0;
				renderContext->draw(render::RpOverlay, renderBlockBegin);

				Vector4 cacheFrameSize(bounds.mn.x, bounds.mn.y, bounds.mx.x, bounds.mx.y);
				Vector4 cacheViewOffset(
					float(slot) / c_cacheSlots,
					0.0f,
					float(c.width) / c_cacheWidth,
					float(c.height) / c_cacheHeight
				);

				m_quad->render(
					renderContext,
					bounds,
					Matrix33::identity(),
					cacheFrameSize,
					cacheViewOffset,
					c_cxfZero,
					0,
					Vector4::zero(),
					false,
					false,
					0
				);

				shape->render(
					renderContext,
					Matrix33::identity(),
					cacheFrameSize,
					cacheViewOffset,
					0.0f,
					c_cxfIdentity,
					false,
					false,
					false,
					SbmDefault
				);

				render::TargetEndRenderBlock* renderBlockEnd = renderContext->alloc< render::TargetEndRenderBlock >("Flash shape render end");
				renderContext->draw(render::RpOverlay, renderBlockEnd);
			}
		}

		if (slot >= 0)
		{
			Vector4 textureOffset(
				float(slot) / c_cacheSlots,
				0.0f,
				float(m_cache[slot].width) / c_cacheWidth,
				float(m_cache[slot].height) / c_cacheHeight
			);

			// Blit shape to frame buffer.
			m_quad->render(
				renderContext,
				bounds,
				transform,
				frameSize,
				viewOffset,
				cxform,
				m_renderTargetShapes->getColorTexture(0),
				textureOffset,
				false,
				false,
				maskReference
			);

			m_cache[slot].unused = 0;
			return;
		}
	}

	// Draw shape directly to framebuffer; either not able to cache or masking.
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
