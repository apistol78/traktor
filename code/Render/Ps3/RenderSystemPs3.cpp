#include <cstdlib>
#include <cell/gcm.h>
#include <sysutil/sysutil_sysparam.h>
#include "Core/Log/Log.h"
#include "Render/DisplayMode.h"
#include "Render/Ps3/Cg.h"
#include "Render/Ps3/CgContext.h"
#include "Render/Ps3/IndexBufferPs3.h"
#include "Render/Ps3/LocalMemoryAllocator.h"
#include "Render/Ps3/RenderSystemPs3.h"
#include "Render/Ps3/RenderTargetSetPs3.h"
#include "Render/Ps3/RenderViewPs3.h"
#include "Render/Ps3/SimpleTexturePs3.h"
#include "Render/Ps3/VertexBufferPs3.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderSystemPs3", RenderSystemPs3, IRenderSystem)

RenderSystemPs3::RenderSystemPs3()
{
}

RenderSystemPs3::~RenderSystemPs3()
{
}

bool RenderSystemPs3::create()
{
	void* hostAddr = std::memalign(1024 * 1024, c_hostSize);
	if (cellGcmInit(c_cbSize, c_hostSize, hostAddr) != CELL_OK)
		return false;

	CellGcmConfig config;
	cellGcmGetConfiguration(&config);
	LocalMemoryAllocator::getInstance().setHeap(config.localAddress, config.localSize);

	return true;
}

void RenderSystemPs3::destroy()
{
	cellGcmSetWaitFlip();
	cellGcmFinish(1);
}

int RenderSystemPs3::getDisplayModeCount() const
{
	return 1;
}

Ref< DisplayMode > RenderSystemPs3::getDisplayMode(int index)
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

	return new DisplayMode(
		videoState.displayMode.resolutionId,
		videoResolution.width,
		videoResolution.height,
		32
	);
}

Ref< DisplayMode > RenderSystemPs3::getCurrentDisplayMode()
{
	return getDisplayMode(0);
}

bool RenderSystemPs3::handleMessages()
{
	return true;
}

Ref< IRenderView > RenderSystemPs3::createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc)
{
	Ref< RenderViewPs3 > renderView = new RenderViewPs3(this);
	if (renderView->create(displayMode, desc))
		return renderView;
	else
		return 0;
}

Ref< IRenderView > RenderSystemPs3::createRenderView(void* windowHandle, const RenderViewCreateDesc& desc)
{
	return 0;
}

Ref< VertexBuffer > RenderSystemPs3::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	void* ptr = LocalMemoryAllocator::getInstance().allocAlign(bufferSize, 128);
	if (!ptr)
		return 0;

	uint32_t offset;
	if (cellGcmAddressToOffset(ptr, &offset) != CELL_OK)
		return 0;

	return new VertexBufferPs3(vertexElements, ptr, offset, bufferSize);
}

Ref< IndexBuffer > RenderSystemPs3::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	void* ptr = LocalMemoryAllocator::getInstance().allocAlign(bufferSize, 128);
	if (!ptr)
		return 0;

	uint32_t offset;
	if (cellGcmAddressToOffset(ptr, &offset) != CELL_OK)
		return 0;

	return new IndexBufferPs3(ptr, offset, indexType, bufferSize);
}

Ref< ISimpleTexture > RenderSystemPs3::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTexturePs3 > texture = new SimpleTexturePs3();
	if (texture->create(desc))
		return texture;
	else
		return 0;
}

Ref< ICubeTexture > RenderSystemPs3::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	return 0;
}

Ref< IVolumeTexture > RenderSystemPs3::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	return 0;
}

Ref< RenderTargetSet > RenderSystemPs3::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetPs3 > renderTargetSet = new RenderTargetSetPs3();
	if (renderTargetSet->create(desc))
		return renderTargetSet;
	else
		return 0;
}

Ref< ProgramResource > RenderSystemPs3::compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate)
{
	CgContext cx(shaderGraph);
	Cg cg;

	if (!cg.generate(cx, shaderGraph))
		return 0;

	std::wstring vertexShader = cx.getVertexShader().getGeneratedShader();
	std::wstring pixelShader = cx.getPixelShader().getGeneratedShader();

	//Ref< ShaderPs3 > shader = gc_new< ShaderPs3 >();
	//if (!shader->create(shaderGraph, vertexShader, pixelShader, cx.getRenderState()))
	//	return 0;

	//return shader;

	return 0;
}

Ref< IProgram > RenderSystemPs3::createProgram(const ProgramResource* programResource)
{
	return 0;
}

	}
}
