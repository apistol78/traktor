/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <gnm.h>
#include <gnmx.h>
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Ps4/ContextPs4.h"
#include "Render/Ps4/CubeTexturePs4.h"
#include "Render/Ps4/IndexBufferDynamicPs4.h"
#include "Render/Ps4/IndexBufferStaticPs4.h"
#include "Render/PS4/MemoryHeapPs4.h"
#include "Render/Ps4/ProgramPs4.h"
#include "Render/Ps4/ProgramCompilerPs4.h"
#include "Render/Ps4/ProgramResourcePs4.h"
#include "Render/Ps4/RenderSystemPs4.h"
#include "Render/Ps4/RenderTargetSetPs4.h"
#include "Render/Ps4/RenderViewPs4.h"
#include "Render/Ps4/SimpleTexturePs4.h"
#include "Render/Ps4/TimeQueryPs4.h"
#include "Render/Ps4/VertexBufferDynamicPs4.h"
#include "Render/Ps4/VertexBufferStaticPs4.h"
#include "Render/Ps4/VolumeTexturePs4.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const size_t c_heapOnionSize = 16UL * 1024UL * 1024UL;
const size_t c_heapOnionAlignment = 64UL * 1024UL;

const size_t c_heapGarlicSize = 64UL * 1024UL * 1024UL;
const size_t c_heapGarlicAlignment = 64UL * 1024UL;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemPs4", 0, RenderSystemPs4, IRenderSystem)

RenderSystemPs4::RenderSystemPs4()
{
}

bool RenderSystemPs4::create(const RenderSystemDesc& desc)
{
	// Initialize ONION heap.
	off_t onionMemOffset = 0;
	void* onionMemPtr = 0;

	sceKernelAllocateDirectMemory(
		0,
		sceKernelGetDirectMemorySize() - 1,
		alignUp(c_heapOnionSize, c_heapOnionAlignment),
		c_heapOnionAlignment,
		SCE_KERNEL_WB_ONION,
		&onionMemOffset
	);

	sceKernelMapDirectMemory(
		&onionMemPtr,
		c_heapOnionSize,
		SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_ALL,
		0,
		onionMemOffset,
		c_heapOnionAlignment
	);

	Ref< MemoryHeapPs4 > heapOnion = new MemoryHeapPs4(onionMemPtr, c_heapOnionSize);

	// Initialize GARLIC heap.
	off_t garlicMemOffset = 0;
	void* garlicMemPtr = 0;

	sceKernelAllocateDirectMemory(
		0,
		sceKernelGetDirectMemorySize() - 1,
		alignUp(c_heapGarlicSize, c_heapGarlicAlignment),
		c_heapGarlicAlignment,
		SCE_KERNEL_WB_GARLIC,
		&garlicMemOffset
	);

	sceKernelMapDirectMemory(
		&garlicMemPtr,
		c_heapGarlicSize,
		SCE_KERNEL_PROT_CPU_WRITE | SCE_KERNEL_PROT_GPU_ALL,
		0,
		garlicMemOffset,
		c_heapGarlicAlignment
	);

	Ref< MemoryHeapPs4 > heapGarlic = new MemoryHeapPs4(garlicMemPtr, c_heapGarlicSize);

	// Create our renderer context.
	m_context = new ContextPs4(
		heapOnion,
		heapGarlic
	);

	return true;
}

void RenderSystemPs4::destroy()
{
	if (m_context)
	{
		m_context->deleteResources();
		m_context = 0;
	}
}

bool RenderSystemPs4::reset(const RenderSystemDesc& desc)
{
	return true;
}

void RenderSystemPs4::getInformation(RenderSystemInformation& outInfo) const
{
	outInfo.dedicatedMemoryTotal = 0;
	outInfo.sharedMemoryTotal = 0;
	outInfo.dedicatedMemoryAvailable = 0;
	outInfo.sharedMemoryAvailable = 0;
}

uint32_t RenderSystemPs4::getDisplayModeCount() const
{
	return 0;
}

DisplayMode RenderSystemPs4::getDisplayMode(uint32_t index) const
{
	DisplayMode dm;
	return dm;
}

DisplayMode RenderSystemPs4::getCurrentDisplayMode() const
{
	DisplayMode cdm;
	return cdm;
}

float RenderSystemPs4::getDisplayAspectRatio() const
{
	return 1.0f;
}

Ref< IRenderView > RenderSystemPs4::createRenderView(const RenderViewDefaultDesc& desc)
{
	Ref< RenderViewPs4 > renderView = new RenderViewPs4(m_context);

	if (!renderView->reset(desc))
		return 0;

	return renderView;
}

Ref< IRenderView > RenderSystemPs4::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	return 0;
}

Ref< VertexBuffer > RenderSystemPs4::createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	if (!dynamic)
		return VertexBufferStaticPs4::create(m_context, bufferSize, vertexElements);
	else
		return VertexBufferDynamicPs4::create(m_context, bufferSize, vertexElements);
}

Ref< IndexBuffer > RenderSystemPs4::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	if (!dynamic)
		return IndexBufferStaticPs4::create(m_context, indexType, bufferSize);
	else
		return IndexBufferDynamicPs4::create(m_context, indexType, bufferSize);
}

Ref< ISimpleTexture > RenderSystemPs4::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTexturePs4 > texture = new SimpleTexturePs4(m_context);
	if (texture->create(desc))
		return texture;
	else
		return 0;
}

Ref< ICubeTexture > RenderSystemPs4::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTexturePs4 > texture = new CubeTexturePs4(m_context);
	if (texture->create(desc))
		return texture;
	else
		return 0;
}

Ref< IVolumeTexture > RenderSystemPs4::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	Ref< VolumeTexturePs4 > texture = new VolumeTexturePs4(m_context);
	if (texture->create(desc))
		return texture;
	else
		return 0;
}

Ref< RenderTargetSet > RenderSystemPs4::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetPs4 > renderTargetSet = new RenderTargetSetPs4(m_context);
	if (renderTargetSet->create(desc))
		return renderTargetSet;
	else
		return 0;
}

Ref< IProgram > RenderSystemPs4::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	Ref< const ProgramResourcePs4 > resource = dynamic_type_cast< const ProgramResourcePs4* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramPs4 > program = new ProgramPs4(m_context);
	if (!program->create(resource))
		return 0;

	return program;
}

Ref< IProgramCompiler > RenderSystemPs4::createProgramCompiler() const
{
	return new ProgramCompilerPs4();
}

Ref< ITimeQuery > RenderSystemPs4::createTimeQuery() const
{
	Ref< TimeQueryPs4 > timeQuery = new TimeQueryPs4(m_context);
	if (timeQuery->create())
		return timeQuery;
	else
		return 0;
}

void RenderSystemPs4::purge()
{
	m_context->deleteResources();
}

void RenderSystemPs4::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

	}
}
