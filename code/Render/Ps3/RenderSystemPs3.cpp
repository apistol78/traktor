#include <cstdlib>
#include <cell/sysmodule.h>
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Thread/Acquire.h"
#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/CubeTexturePs3.h"
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
#include "Render/Ps3/VolumeTexturePs3.h"

using namespace cell::Gcm;

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_cbSize = 256 * 1024;
const uint32_t c_hostSize = 1 * 1024 * 1024;
const uint32_t c_mainSize = 64 * 1024 * 1024;	//< RSX mapped main memory; used for dynamic index- and vertexbuffers.

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemPs3", 0, RenderSystemPs3, IRenderSystem)

RenderSystemPs3::RenderSystemPs3()
:	m_tileArea(15, 2047)
,	m_zcullArea(8, 3145728)
,	m_counterVertexBuffers(0)
,	m_counterIndexBuffers(0)
,	m_counterSimpleTextures(0)
,	m_counterRenderTargetSets(0)
,	m_counterPrograms(0)
{
}

RenderSystemPs3::~RenderSystemPs3()
{
}

bool RenderSystemPs3::create(const RenderSystemDesc& desc)
{
	int32_t err;

	// Load A/V configuration module; necessary in order to change gamma.
	cellSysmoduleLoadModule(CELL_SYSMODULE_AVCONF_EXT);

	uint8_t* data = (uint8_t*)Alloc::acquireAlign(c_hostSize + c_mainSize, 1024 * 1024, T_FILE_LINE);
	if (!data)
		return 0;

	void* hostAddr = data;
	void* mainAddr = data + c_hostSize;

	if (cellGcmInit(c_cbSize, c_hostSize, hostAddr) != CELL_OK)
		return false;

	CellGcmConfig config;
	cellGcmGetConfiguration(&config);
	m_memoryHeapLocal = new MemoryHeap(
		config.localAddress,
		config.localSize,
		0/*64 * 1024 * 1024*/,
		CELL_GCM_LOCATION_LOCAL
	);

	uint32_t status;
	if (cellGcmMapMainMemory(mainAddr, c_mainSize, &status) != CELL_OK)
		return false;

	m_memoryHeapMain = new MemoryHeap(
		mainAddr,
		c_mainSize,
		0,
		CELL_GCM_LOCATION_MAIN
	);

	log::info << 
		L"PS3 render system created" << Endl <<
		L"\tLocal address 0x" << config.localAddress << Endl <<
		L"\t      size " << config.localSize / (1024 * 1024) << L" MiB" << Endl <<
		L"\tMain address 0x" << mainAddr << Endl <<
		L"\t      size " << c_mainSize / (1024 * 1024) << L" MiB" << Endl;

	// Get video state.
	CellVideoOutState videoState;
	for (;;)
	{
		err = cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
		if (err != CELL_VIDEO_OUT_ERROR_CONDITION_BUSY)
			break;
		sys_timer_sleep(1);
	}

#if defined(_DEBUG)
	log::debug << L"Supported video output refresh rate(s):";
	if (videoState.displayMode.refreshRates & CELL_VIDEO_OUT_REFRESH_RATE_59_94HZ)
		log::debug << L" 59.94Hz";
	if (videoState.displayMode.refreshRates & CELL_VIDEO_OUT_REFRESH_RATE_50HZ)
		log::debug << L" 50Hz";
	if (videoState.displayMode.refreshRates & CELL_VIDEO_OUT_REFRESH_RATE_60HZ)
		log::debug << L" 60Hz";
	if (videoState.displayMode.refreshRates & CELL_VIDEO_OUT_REFRESH_RATE_30HZ)
		log::debug << L" 30Hz";
	log::debug << Endl;
#endif

	// Determine supported display modes.
	for (const ResolutionDesc* i = c_resolutionDescs; i->id; ++i)
	{
		if (!i->stereoscopic && !(i->refreshRates & videoState.displayMode.refreshRates))
		{
			log::debug << L"Skipping unsupported display mode " << i->width << L"*" << i->height << Endl;
			continue;
		}

		if (cellVideoOutGetResolutionAvailability(CELL_VIDEO_OUT_PRIMARY, i->id, CELL_VIDEO_OUT_ASPECT_AUTO, 0))
			m_resolutions.push_back(i);
	}

	return true;
}

void RenderSystemPs3::destroy()
{
	//cellGcmSetWaitFlip();
	//cellGcmFinish(1);
}

bool RenderSystemPs3::reset(const RenderSystemDesc& desc)
{
	return false;
}

void RenderSystemPs3::getInformation(RenderSystemInformation& outInfo) const
{
}

uint32_t RenderSystemPs3::getDisplayModeCount() const
{
	return m_resolutions.size();
}

DisplayMode RenderSystemPs3::getDisplayMode(uint32_t index) const
{
	DisplayMode dm;
	dm.width = m_resolutions[index]->width;
	dm.height = m_resolutions[index]->height;
	dm.refreshRate = 0;
	dm.colorBits = 24;
	dm.stereoscopic = m_resolutions[index]->stereoscopic;
	return dm;
}

DisplayMode RenderSystemPs3::getCurrentDisplayMode() const
{
	CellVideoOutState videoState;
	CellVideoOutResolution videoResolution;
	int32_t err;

	for (;;)
	{
		err = cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
		if (err != CELL_VIDEO_OUT_ERROR_CONDITION_BUSY)
			break;
		sys_timer_sleep(1);
	}
	
	if (err != CELL_OK)
	{
		log::error << L"Unable to get video output state (" << lookupGcmError(err) << L")" << Endl;
		return DisplayMode();
	}

	err = cellVideoOutGetResolution(videoState.displayMode.resolutionId, &videoResolution);
	if (err != CELL_OK)
	{
		log::error << L"Unable to get resolution description (" << lookupGcmError(err) << L")" << Endl;
		return DisplayMode();
	}

	// Need to ensure 50Hz output if 576i display mode; otherwise
	// we fall back to 480i as we're not in PAL region.
	if (videoResolution.height == 576)
	{
		if (!(videoState.displayMode.refreshRates & CELL_VIDEO_OUT_REFRESH_RATE_50HZ))
		{
			log::debug << L"576i resolution only available on PAL; using 480i instead" << Endl;
			videoResolution.width = 640;
			videoResolution.height = 480;
		}
	}

	DisplayMode dm;
	dm.width = videoResolution.width;
	dm.height = videoResolution.height;
	dm.refreshRate = 0;
	dm.colorBits = 24;
	dm.stereoscopic = false;
	return dm;
}

float RenderSystemPs3::getDisplayAspectRatio() const
{
	CellVideoOutState videoState;
	int32_t err;

	for (;;)
	{
		err = cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
		if (err != CELL_VIDEO_OUT_ERROR_CONDITION_BUSY)
			break;
		sys_timer_sleep(1);
	}

	if (err != CELL_OK)
	{
		log::error << L"Unable to get video output state (" << lookupGcmError(err) << L")" << Endl;
		return 0.0f;
	}

	switch (videoState.displayMode.aspect)
	{
	case CELL_VIDEO_OUT_ASPECT_4_3:
		return 4.0f / 3.0f;
	case CELL_VIDEO_OUT_ASPECT_16_9:
		return 16.0f / 9.0f;
	}

	return 0.0f;
}

Ref< IRenderView > RenderSystemPs3::createRenderView(const RenderViewDefaultDesc& desc)
{
	Ref< RenderViewPs3 > renderView = new RenderViewPs3(this, m_memoryHeapLocal, m_memoryHeapMain, m_tileArea, m_zcullArea);
	if (renderView->create(desc))
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
	MemoryHeapObject* vbo = (dynamic ? m_memoryHeapMain : m_memoryHeapLocal)->alloc(bufferSize, 16, false);
	if (vbo)
		return new VertexBufferPs3(vertexElements, vbo, bufferSize, m_counterVertexBuffers);
	else
		return 0;
}

Ref< IndexBuffer > RenderSystemPs3::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	MemoryHeapObject* ibo = (dynamic ? m_memoryHeapMain : m_memoryHeapLocal)->alloc(bufferSize, 16, false);
	if (ibo)
		return new IndexBufferPs3(ibo, indexType, bufferSize, m_counterIndexBuffers);
	else
		return 0;
}

Ref< ISimpleTexture > RenderSystemPs3::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTexturePs3 > texture = new SimpleTexturePs3(m_counterSimpleTextures);
	if (texture->create(m_memoryHeapLocal, desc))
		return texture;
	else
		return 0;
}

Ref< ICubeTexture > RenderSystemPs3::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTexturePs3 > texture = new CubeTexturePs3(m_counterSimpleTextures);
	if (texture->create(m_memoryHeapLocal, desc))
		return texture;
	else
		return 0;
}

Ref< IVolumeTexture > RenderSystemPs3::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	Ref< VolumeTexturePs3 > texture = new VolumeTexturePs3(m_counterSimpleTextures);
	if (texture->create(m_memoryHeapLocal, desc))
		return texture;
	else
		return 0;
}

Ref< RenderTargetSet > RenderSystemPs3::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	Ref< RenderTargetSetPs3 > renderTargetSet = new RenderTargetSetPs3(m_tileArea, m_zcullArea, m_counterRenderTargetSets);
	if (renderTargetSet->create(m_memoryHeapLocal, desc))
		return renderTargetSet;
	else
		return 0;
}

Ref< IProgram > RenderSystemPs3::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	Ref< const ProgramResourcePs3 > resource = dynamic_type_cast< const ProgramResourcePs3* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramPs3 > program = new ProgramPs3(m_counterPrograms);
	if (!program->create(m_memoryHeapLocal, m_memoryHeapMain, resource))
		return 0;

	return program;
}

Ref< IProgramCompiler > RenderSystemPs3::createProgramCompiler() const
{
#if !defined(_PS3)
	return new ProgramCompilerPs3();
#else
	return 0;
#endif
}

Ref< ITimeQuery > RenderSystemPs3::createTimeQuery() const
{
	return 0;
}

void RenderSystemPs3::purge()
{
}

void RenderSystemPs3::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

void RenderSystemPs3::beginRendering()
{
	while (!m_lock.wait());

	m_memoryHeapLocal->compact();
	m_memoryHeapMain->compact();
}

void RenderSystemPs3::endRendering()
{
	m_lock.release();
}

	}
}
