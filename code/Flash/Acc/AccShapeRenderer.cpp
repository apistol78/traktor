#include <GuillotineBinPack.h>
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

const Matrix33 c_flipped(
	0.0f, 1, 0,
	1, 0, 0,
	0, 0, 1
);

const uint32_t c_cacheWidth = 2048;
const uint32_t c_cacheHeight = 2048;

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

	m_quadCount = 0;
	m_shapeCount = 0;
}

void AccShapeRenderer::endFrame()
{
	//log::info << L"quad = " << m_quadCount << L", shape = " << m_shapeCount << Endl;
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

		if (slot < 0)
		{
			rbp::Rect node = m_packer->Insert(pixelWidth, pixelHeight, false, rbp::GuillotineBinPack::RectBestAreaFit, rbp::GuillotineBinPack::SplitShorterLeftoverAxis);
			if (node.width > 0 && node.height > 0)
			{
				Cache c;
				c.tag = tag;
				c.x = node.x;
				c.y = node.y;
				c.width = node.width;
				c.height = node.height;
				c.flipped = bool(node.width != pixelWidth);
				c.unused = 0;
				
				slot = m_cache.size();
				m_cache.push_back(c);

				// Cache shape into off-screen target.
				render::TargetBeginRenderBlock* renderBlockBegin = renderContext->alloc< render::TargetBeginRenderBlock >("Flash shape render begin");
				renderBlockBegin->renderTargetSet = m_renderTargetShapes;
				renderBlockBegin->renderTargetIndex = 0;
				renderContext->draw(render::RpOverlay, renderBlockBegin);

				Vector4 cacheFrameSize(bounds.mn.x, bounds.mn.y, bounds.mx.x, bounds.mx.y);
				Vector4 cacheViewOffset(
					float(c.x) / c_cacheWidth,
					float(c.y) / c_cacheHeight,
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
					c.flipped ? c_flipped : Matrix33::identity(),
					c.flipped ? Vector4(bounds.mn.y, bounds.mn.x, bounds.mx.y, bounds.mx.x) : cacheFrameSize,
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
				float(m_cache[slot].x) / c_cacheWidth,
				float(m_cache[slot].y) / c_cacheHeight,
				float(m_cache[slot].width) / c_cacheWidth,
				float(m_cache[slot].height) / c_cacheHeight
			);

			// Blit shape to frame buffer.
			m_quad->render(
				renderContext,
				m_cache[slot].flipped ? Aabb2(bounds.mn.shuffle< 1, 0 >(), bounds.mx.shuffle< 1, 0 >()) : bounds,
				transform * (m_cache[slot].flipped ? c_flipped : Matrix33::identity()),
				frameSize,
				viewOffset,
				cxform,
				m_renderTargetShapes->getColorTexture(0),
				textureOffset,
				false,
				false,
				maskReference
			);

			m_quadCount++;
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

	m_shapeCount++;
}

	}
}
