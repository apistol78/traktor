#include <video_out.h>
#include "Core/Misc/SafeDestroy.h"
#include "Render/Ps4/ContextPs4.h"
#include "Render/Ps4/RenderQueuePs4.h"
#include "Render/Ps4/RenderTargetDepthPs4.h"
#include "Render/Ps4/RenderTargetPs4.h"
#include "Render/Ps4/RenderTargetSetPs4.h"
#include "Render/Ps4/RenderViewPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewPs4", RenderViewPs4, IRenderView)

RenderViewPs4::RenderViewPs4(ContextPs4* context)
:	m_context(context)
,	m_videoOutHandle(0)
,	m_currentQueue(0)
,	m_width(0)
,	m_height(0)
{
}

RenderViewPs4::~RenderViewPs4()
{
	close();
}

bool RenderViewPs4::create()
{
	// Create render queues.
	for (int32_t i = 0; i < FrameQueueCount; ++i)
	{
		m_queues[0] = new RenderQueuePs4(m_context);
		if (!m_queues[0]->create())
			return false;
	}

	// Create end-of-pipe event.
	sceKernelCreateEqueue(&m_eopEventQueue, "EOP queue");
	sce::Gnm::addEqEvent(m_eopEventQueue, sce::Gnm::kEqEventGfxEop, nullptr);

	return true;
}

bool RenderViewPs4::nextEvent(RenderEvent& outEvent)
{
	return false;
}

void RenderViewPs4::close()
{
	for (int32_t i = 0; i < FrameQueueCount; ++i)
		safeDestroy(m_queues[i]);
}

bool RenderViewPs4::reset(const RenderViewDefaultDesc& desc)
{
	// Close video output if already opened.
	if (m_videoOutHandle != 0)
	{
		sceVideoOutClose(m_videoOutHandle);
		m_videoOutHandle = 0;
	}

	for (int32_t i = 0; i < FrameQueueCount; ++i)
		safeDestroy(m_primaryTargets[i]);

	m_width = desc.displayMode.width;
	m_height = desc.displayMode.height;

	// Create primary render targets.
	RenderTargetSetCreateDesc rtscd;
	rtscd.count = 1;
	rtscd.width = m_width;
	rtscd.height = m_height;
	rtscd.multiSample = 0;
	rtscd.createDepthStencil = true;
	rtscd.usingDepthStencilAsTexture = false;
	rtscd.usingPrimaryDepthStencil = false;
	rtscd.preferTiled = false;
	rtscd.ignoreStencil = false;
	rtscd.generateMips = false;
	rtscd.targets[0].format = TfR8G8B8A8;
	rtscd.targets[0].sRGB = true;

	for (int32_t i = 0; i < FrameQueueCount; ++i)
	{
		m_primaryTargets[i] = new RenderTargetSetPs4(m_context);
		if (!m_primaryTargets[i]->create(rtscd))
			return false;
	}

	m_videoOutHandle = sceVideoOutOpen(0, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, NULL);

	// Set up the initialization parameters for the VideoOut library.
	SceVideoOutBufferAttribute videoOutBufferAttribute;
	sceVideoOutSetBufferAttribute(
		&videoOutBufferAttribute,
		SCE_VIDEO_OUT_PIXEL_FORMAT_B8_G8_R8_A8_SRGB,
		SCE_VIDEO_OUT_TILING_MODE_TILE,
		SCE_VIDEO_OUT_ASPECT_RATIO_16_9,
		m_width,
		m_height,
		m_primaryTargets[0]->getColorTargetPs4(0)->getRenderTargetGnm().getPitch()
	);

	// Register the display buffers to the slot: [0..kDisplayBufferCount-1].
	void* targetAddresses[] = { 0, 0, 0, 0 };
	for (int32_t i = 0; i < FrameQueueCount; ++i)
		targetAddresses[i] = m_primaryTargets[i]->getColorTargetPs4(0)->getRenderTargetGnm().getBaseAddress();

	sceVideoOutRegisterBuffers(
		m_videoOutHandle,
		0,
		targetAddresses,
		FrameQueueCount,
		&videoOutBufferAttribute
	);

	// Initialize the flip rate: 0: 60Hz, 1: 30Hz or 2: 20Hz.
	sceVideoOutSetFlipRate(m_videoOutHandle, 0);
	return true;
}

bool RenderViewPs4::reset(int32_t width, int32_t height)
{
	return false;
}

int RenderViewPs4::getWidth() const
{
	return m_width;
}

int RenderViewPs4::getHeight() const
{
	return m_height;
}

bool RenderViewPs4::isActive() const
{
	return true;
}

bool RenderViewPs4::isFullScreen() const
{
	return true;
}

void RenderViewPs4::showCursor()
{
}

void RenderViewPs4::hideCursor()
{
}

bool RenderViewPs4::isCursorVisible() const
{
	return false;
}

bool RenderViewPs4::setGamma(float gamma)
{
	return false;
}

void RenderViewPs4::setViewport(const Viewport& viewport)
{
}

Viewport RenderViewPs4::getViewport()
{
	return Viewport();
}

SystemWindow RenderViewPs4::getSystemWindow()
{
	SystemWindow sw;
	return sw;
}

bool RenderViewPs4::begin(EyeType eye)
{
	RenderQueuePs4* queue = m_queues[m_currentQueue];
	T_ASSERT (queue);

	sce::Gnmx::GfxContext& gfxContext = queue->getGfxContext();

	RenderTargetSetPs4* primaryTarget = m_primaryTargets[m_currentQueue];
	T_ASSERT (primaryTarget);

	// Wait until the context label has been written to make sure that the
	// GPU finished parsing the command buffers before overwriting them
	while (*queue->getEopLabel() != RenderQueuePs4::EopsFinished)
	{
		SceKernelEvent eopEvent; int count;
		sceKernelWaitEqueue(m_eopEventQueue, &eopEvent, 1, &count, nullptr);
	}

	// Reset the EOP and flip GPU labels
	*queue->getEopLabel() = RenderQueuePs4::EopsNotYet;
	*queue->getContextLabel() = RenderQueuePs4::RcsInUse;

	// Reset the graphical context and initialize the hardware state.
	gfxContext.reset();
	gfxContext.initializeDefaultHardwareState();

	// The waitUntilSafeForRendering stalls the GPU until the scan-out
	// operations on the current display buffer have been completed.
	gfxContext.waitUntilSafeForRendering(
		m_videoOutHandle,
		0/*backBuffer->displayIndex*/
	);

	// Setup the viewport to match the entire screen.
	// The z-scale and z-offset values are used to specify the transformation
	// from clip-space to screen-space
	gfxContext.setupScreenViewport(
		0,
		0,
		m_width,
		m_height,
		0.5f,	// Z-scale
		0.5f	// Z-offset
	);

	// Bind the render & depth targets to the context.
	gfxContext.setRenderTarget(0, &primaryTarget->getColorTargetPs4(0)->getRenderTargetGnm());
	gfxContext.setDepthRenderTarget(&primaryTarget->getDepthTargetPs4()->getDepthTargetGnm());
	return true;
}

bool RenderViewPs4::begin(RenderTargetSet* renderTargetSet)
{
	return true;
}

bool RenderViewPs4::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	return true;
}

void RenderViewPs4::clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil)
{
}

void RenderViewPs4::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
}

void RenderViewPs4::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
}

void RenderViewPs4::end()
{
}

void RenderViewPs4::present()
{
	RenderQueuePs4* queue = m_queues[m_currentQueue];
	T_ASSERT (queue);

	sce::Gnmx::GfxContext& gfxContext = queue->getGfxContext();

	// Write the label that indicates that the GPU finished working on this frame
	// and trigger a software interrupt to signal the EOP event queue
	gfxContext.writeAtEndOfPipeWithInterrupt(
		sce::Gnm::kEopFlushCbDbCaches,
		sce::Gnm::kEventWriteDestMemory,
		const_cast< uint32_t* >(queue->getEopLabel()),
		sce::Gnm::kEventWriteSource32BitsImmediate,
		RenderQueuePs4::EopsFinished,
		sce::Gnm::kCacheActionNone,
		sce::Gnm::kCachePolicyLru
	);

	// Submit the command buffers, request a flip of the display buffer and
	// write the GPU label that determines the render context state (free)
	//
	// NOTE: for this basic sample we are submitting a single GfxContext
	// per frame. Submitting multiple GfxContext-s per frame is allowed.
	// Multiple contexts are processed in order, i.e.: they start in
	// submission order and end in submission order.
	int32_t ret = gfxContext.submitAndFlip(
		m_videoOutHandle,
		0/*backBuffer->displayIndex*/,
		SCE_VIDEO_OUT_FLIP_MODE_VSYNC,
		0,
		const_cast< uint32_t* >(queue->getContextLabel()),
		RenderQueuePs4::RcsFree
	);
	if (ret != sce::Gnm::kSubmissionSuccess)
	{
		// Analyze the error code to determine whether the command buffers
		// have been submitted to the GPU or not
		if (ret & sce::Gnm::kStatusMaskError)
		{
			// Error codes in the kStatusMaskError family block submissions
			// so we need to mark this render context as not-in-flight
			*queue->getEopLabel() = RenderQueuePs4::EopsFinished;
			*queue->getContextLabel() = RenderQueuePs4::RcsFree;
		}
	}

	m_currentQueue = (m_currentQueue + 1) % FrameQueueCount;
}

void RenderViewPs4::pushMarker(const char* const marker)
{
}

void RenderViewPs4::popMarker()
{
}

void RenderViewPs4::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = 0;
	outStatistics.primitiveCount = 0;
}

bool RenderViewPs4::getBackBufferContent(void* buffer) const
{
	return false;
}

	}
}
