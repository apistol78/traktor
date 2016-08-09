#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/CubeTextureVk.h"
#include "Render/Vulkan/IndexBufferDynamicVk.h"
#include "Render/Vulkan/IndexBufferStaticVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/ProgramCompilerVk.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/RenderSystemVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/RenderViewVk.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/TimeQueryVk.h"
#include "Render/Vulkan/VertexBufferDynamicVk.h"
#include "Render/Vulkan/VertexBufferStaticVk.h"
#include "Render/Vulkan/VolumeTextureVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemVk", 0, RenderSystemVk, IRenderSystem)

RenderSystemVk::RenderSystemVk()
{
}

bool RenderSystemVk::create(const RenderSystemDesc& desc)
{
	return true;
}

void RenderSystemVk::destroy()
{
}

bool RenderSystemVk::reset(const RenderSystemDesc& desc)
{
	return true;
}

void RenderSystemVk::getInformation(RenderSystemInformation& outInfo) const
{
	outInfo.dedicatedMemoryTotal = 0;
	outInfo.sharedMemoryTotal = 0;
	outInfo.dedicatedMemoryAvailable = 0;
	outInfo.sharedMemoryAvailable = 0;
}

uint32_t RenderSystemVk::getDisplayModeCount() const
{
	return 0;
}

DisplayMode RenderSystemVk::getDisplayMode(uint32_t index) const
{
	DisplayMode dm;
	return dm;
}

DisplayMode RenderSystemVk::getCurrentDisplayMode() const
{
	DisplayMode cdm;
	return cdm;
}

float RenderSystemVk::getDisplayAspectRatio() const
{
	return 1.0f;
}

Ref< IRenderView > RenderSystemVk::createRenderView(const RenderViewDefaultDesc& desc)
{
	Ref< RenderViewVk > renderView = new RenderViewVk();

	if (!renderView->reset(desc))
		return 0;

	return renderView;
}

Ref< IRenderView > RenderSystemVk::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	return 0;
}

Ref< VertexBuffer > RenderSystemVk::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	if (!dynamic)
		return VertexBufferStaticVk::create(bufferSize, vertexElements);
	else
		return VertexBufferDynamicVk::create(bufferSize, vertexElements);
}

Ref< IndexBuffer > RenderSystemVk::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	if (!dynamic)
		return IndexBufferStaticVk::create(indexType, bufferSize);
	else
		return IndexBufferDynamicVk::create(indexType, bufferSize);
}

Ref< ISimpleTexture > RenderSystemVk::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureVk > texture = new SimpleTextureVk();
	if (texture->create(desc))
		return texture;
	else
		return 0;
}

Ref< ICubeTexture > RenderSystemVk::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTextureVk > texture = new CubeTextureVk();
	if (texture->create(desc))
		return texture;
	else
		return 0;
}

Ref< IVolumeTexture > RenderSystemVk::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	Ref< VolumeTextureVk > texture = new VolumeTextureVk();
	if (texture->create(desc))
		return texture;
	else
		return 0;
}

Ref< RenderTargetSet > RenderSystemVk::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetVk > renderTargetSet = new RenderTargetSetVk();
	if (renderTargetSet->create(desc))
		return renderTargetSet;
	else
		return 0;
}

Ref< IProgram > RenderSystemVk::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	Ref< const ProgramResourceVk > resource = dynamic_type_cast< const ProgramResourceVk* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramVk > program = new ProgramVk();
	if (!program->create(resource))
		return 0;

	return program;
}

Ref< IProgramCompiler > RenderSystemVk::createProgramCompiler() const
{
	return new ProgramCompilerVk();
}

Ref< ITimeQuery > RenderSystemVk::createTimeQuery() const
{
	Ref< TimeQueryVk > timeQuery = new TimeQueryVk();
	if (timeQuery->create())
		return timeQuery;
	else
		return 0;
}

void RenderSystemVk::purge()
{
}

void RenderSystemVk::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

	}
}
