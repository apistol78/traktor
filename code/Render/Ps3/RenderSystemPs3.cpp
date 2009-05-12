#include <cstdlib>
#include <cell/gcm.h>
#include <sysutil/sysutil_sysparam.h>
#include "Render/Ps3/RenderSystemPs3.h"
#include "Render/Ps3/RenderViewPs3.h"
#include "Render/Ps3/VertexBufferPs3.h"
#include "Render/Ps3/IndexBufferPs3.h"
#include "Render/Ps3/SimpleTexturePs3.h"
#include "Render/Ps3/ShaderPs3.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/CgContext.h"
#include "Render/Ps3/Cg.h"
#include "Render/Ps3/LocalMemoryAllocator.h"
#include "Render/DisplayMode.h"
#include "Core/Log/Log.h"

using namespace cell::Gcm;

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_cbSize = 0x10000;
const uint32_t c_hostSize = 1 * 1024 * 1024;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderSystemPs3", RenderSystemPs3, RenderSystem)

RenderSystemPs3::RenderSystemPs3()
{
	void* hostAddr = std::memalign(1024 * 1024, c_hostSize);
	if (cellGcmInit(c_cbSize, c_hostSize, hostAddr) != CELL_OK)
	{
		log::error << L"GCM init failed, program halted!" << Endl;
		for (;;);
	}

	CellGcmConfig config;
	cellGcmGetConfiguration(&config);
	LocalMemoryAllocator::getInstance().setHeap(config.localAddress, config.localSize);
}

RenderSystemPs3::~RenderSystemPs3()
{
	cellGcmSetWaitFlip();
	cellGcmFinish(1);
}

int RenderSystemPs3::getDisplayModeCount() const
{
	return 1;
}

DisplayMode* RenderSystemPs3::getDisplayMode(int index)
{
	CellVideoOutState videoState;
	CellVideoOutResolution videoResolution;
	int32_t ret;

	ret = cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
	if (ret != CELL_OK)
	{
		log::error << L"Get display mode failed, cellVideoOutGetState failed" << Endl;
		return 0;
	}

	cellVideoOutGetResolution(videoState.displayMode.resolutionId, &videoResolution);

	return gc_new< DisplayMode >(
		videoState.displayMode.resolutionId,
		videoResolution.width,
		videoResolution.height,
		32
	);
}

DisplayMode* RenderSystemPs3::getCurrentDisplayMode()
{
	return getDisplayMode(0);
}

bool RenderSystemPs3::handleMessages()
{
	return true;
}

RenderView* RenderSystemPs3::createRenderView(DisplayMode* displayMode, int depthBits, int stencilBits, int multiSample, bool waitVBlank)
{
	Ref< RenderViewPs3 > renderView = gc_new< RenderViewPs3 >(this);
	if (!renderView->create(displayMode->getIndex(), displayMode->getWidth(), displayMode->getHeight()))
		return 0;

	return renderView;
}

RenderView* RenderSystemPs3::createRenderView(void* windowHandle, int depthBits, int stencilBits, int multiSample)
{
	return 0;
}

VertexBuffer* RenderSystemPs3::createVertexBuffer(const std::vector< VertexElement >& vertexElements, int bufferSize, bool dynamic)
{
	void* ptr = LocalMemoryAllocator::getInstance().allocAlign(bufferSize, 128);
	if (!ptr)
		return 0;

	uint32_t offset;
	if (cellGcmAddressToOffset(ptr, &offset) != CELL_OK)
		return 0;

	return gc_new< VertexBufferPs3 >(vertexElements, ptr, offset, bufferSize);
}

IndexBuffer* RenderSystemPs3::createIndexBuffer(IndexType indexType, int bufferSize, bool dynamic)
{
	void* ptr = LocalMemoryAllocator::getInstance().allocAlign(bufferSize, 128);
	if (!ptr)
		return 0;

	uint32_t offset;
	if (cellGcmAddressToOffset(ptr, &offset) != CELL_OK)
		return 0;

	return gc_new< IndexBufferPs3 >(ptr, offset, indexType, bufferSize);
}

SimpleTexture* RenderSystemPs3::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTexturePs3 > texture = gc_new< SimpleTexturePs3 >();
	if (!texture->create(desc))
		return 0;

	return texture;
}

CubeTexture* RenderSystemPs3::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	return 0;
}

VolumeTexture* RenderSystemPs3::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	return 0;
}

RenderTarget* RenderSystemPs3::createRenderTarget(const RenderTargetCreateDesc& desc)
{
	Ref< RenderTargetPs3 > renderTarget = gc_new< RenderTargetPs3 >();
	if (!renderTarget->create(desc))
		return 0;

	return renderTarget;
}

Shader* RenderSystemPs3::createShader(ShaderGraph* shaderGraph)
{
	CgContext cx(shaderGraph);
	Cg cg;

	if (!cg.generate(cx, shaderGraph))
		return 0;

	std::wstring vertexShader = cx.getVertexShader().getGeneratedShader();
	std::wstring pixelShader = cx.getPixelShader().getGeneratedShader();

	Ref< ShaderPs3 > shader = gc_new< ShaderPs3 >();
	if (!shader->create(shaderGraph, vertexShader, pixelShader, cx.getRenderState()))
		return 0;

	return shader;
}

	}
}
