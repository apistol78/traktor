#include <cstdlib>
#include <cell/sysmodule.h>
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Thread/Acquire.h"
#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/IndexBufferPs3.h"
#include "Render/Ps3/MemoryHeap.h"
#include "Render/Ps3/ProgramPs3.h"
#include "Render/Ps3/ProgramCompilerPs3.h"
#include "Render/Ps3/ProgramResourcePs3.h"
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

const uint32_t c_cbSize = 256 * 1024;
const uint32_t c_hostSize = 8 * 1024 * 1024;
const uint32_t c_mainSize = 8 * 1024 * 1024;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemPs3", 0, RenderSystemPs3, IRenderSystem)

RenderSystemPs3::RenderSystemPs3()
{
}

RenderSystemPs3::~RenderSystemPs3()
{
}

bool RenderSystemPs3::create(const RenderSystemCreateDesc& desc)
{
	// Load A/V configuration module; necessary in order to change gamma.
	cellSysmoduleLoadModule(CELL_SYSMODULE_AVCONF_EXT);

	uint8_t* data = (uint8_t*)Alloc::acquireAlign(c_hostSize + c_mainSize, 1024 * 1024);
	if (!data)
		return 0;

	void* hostAddr = data;
	void* mainAddr = data + c_hostSize;

	if (cellGcmInit(c_cbSize, c_hostSize, hostAddr) != CELL_OK)
		return false;

	CellGcmConfig config;
	cellGcmGetConfiguration(&config);
	m_memoryHeapLocal = new MemoryHeap(config.localAddress, config.localSize, CELL_GCM_LOCATION_LOCAL);

	uint32_t status;
	if (cellGcmMapMainMemory(mainAddr, c_mainSize, &status) != CELL_OK)
		return false;

	m_memoryHeapMain = new MemoryHeap(mainAddr, c_mainSize, CELL_GCM_LOCATION_MAIN);

	log::info << 
		L"PS3 render system created" << Endl <<
		L"\tLocal address 0x" << config.localAddress << Endl <<
		L"\t      size " << config.localSize / (1024 * 1024) << L" MiB" << Endl <<
		L"\tMain address 0x" << mainAddr << Endl <<
		L"\t      size " << c_mainSize / (1024 * 1024) << L" MiB" << Endl;

	return true;
}

void RenderSystemPs3::destroy()
{
	cellGcmSetWaitFlip();
	cellGcmFinish(1);
}

uint32_t RenderSystemPs3::getDisplayModeCount() const
{
	return 1;
}

DisplayMode RenderSystemPs3::getDisplayMode(uint32_t index) const
{
	CellVideoOutState videoState;
	CellVideoOutResolution videoResolution;
	int32_t ret;

	ret = cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
	if (ret != CELL_OK)
		log::error << L"Get display mode failed, cellVideoOutGetState failed" << Endl;

	cellVideoOutGetResolution(videoState.displayMode.resolutionId, &videoResolution);

	DisplayMode dm;
	dm.width = videoResolution.width;
	dm.height = videoResolution.height;
	dm.refreshRate = 0;
	dm.colorBits = 0;
	return dm;
}

DisplayMode RenderSystemPs3::getCurrentDisplayMode() const
{
	return getDisplayMode(0);
}

bool RenderSystemPs3::handleMessages()
{
	return true;
}

Ref< IRenderView > RenderSystemPs3::createRenderView(const RenderViewDefaultDesc& desc)
{
	Ref< RenderViewPs3 > renderView = new RenderViewPs3(this);
	if (renderView->create(m_memoryHeapLocal, desc))
		return renderView;
	else
		return 0;
}

Ref< IRenderView > RenderSystemPs3::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	return 0;
}

Ref< VertexBuffer > RenderSystemPs3::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	
	MemoryHeapObject* vbo = (dynamic ? m_memoryHeapMain : m_memoryHeapLocal)->alloc(bufferSize, 16, false);

	if (vbo)
		return new VertexBufferPs3(vertexElements, vbo, bufferSize);
	else
		return 0;
}

Ref< IndexBuffer > RenderSystemPs3::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	
	MemoryHeapObject* ibo = (dynamic ? m_memoryHeapMain : m_memoryHeapLocal)->alloc(bufferSize, 16, false);

	if (ibo)
		return new IndexBufferPs3(ibo, indexType, bufferSize);
	else
		return 0;
}

Ref< ISimpleTexture > RenderSystemPs3::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	Ref< SimpleTexturePs3 > texture = new SimpleTexturePs3();
	if (texture->create(m_memoryHeapLocal, desc))
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	Ref< RenderTargetSetPs3 > renderTargetSet = new RenderTargetSetPs3();
	if (renderTargetSet->create(m_memoryHeapLocal, desc))
		return renderTargetSet;
	else
		return 0;
}

Ref< IProgram > RenderSystemPs3::createProgram(const ProgramResource* programResource)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	Ref< const ProgramResourcePs3 > resource = dynamic_type_cast< const ProgramResourcePs3* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramPs3 > program = new ProgramPs3();
	if (!program->create(m_memoryHeapLocal, resource))
		return 0;

	return program;
}

Ref< IProgramCompiler > RenderSystemPs3::createProgramCompiler() const
{
	return new ProgramCompilerPs3();
}

void RenderSystemPs3::compactHeaps()
{
	m_memoryHeapLocal->compact();
	m_memoryHeapMain->compact();
}

	}
}
