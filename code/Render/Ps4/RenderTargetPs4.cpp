#include "Render/Types.h"
#include "Render/Ps4/ContextPs4.h"
#include "Render/Ps4/MemoryHeapPs4.h"
#include "Render/Ps4/MemoryHeapObjectPs4.h"
#include "Render/Ps4/RenderTargetPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetPs4", RenderTargetPs4, ISimpleTexture)

RenderTargetPs4::RenderTargetPs4(ContextPs4* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
{
}

RenderTargetPs4::~RenderTargetPs4()
{
	destroy();
}

bool RenderTargetPs4::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc)
{
	sce::Gnm::TileMode tileMode;
	sce::Gnm::DataFormat format = sce::Gnm::kDataFormatB8G8R8A8UnormSrgb;

	sce::GpuAddress::computeSurfaceTileMode(
		&tileMode,
		sce::GpuAddress::kSurfaceTypeColorTargetDisplayable,
		format,
		1
	);

	const sce::Gnm::SizeAlign sizeAndAlign = m_renderTarget.init(
		setDesc.width,
		setDesc.height,
		1,
		format,
		tileMode,
		sce::Gnm::kNumSamples1,
		sce::Gnm::kNumFragments1,
		nullptr,
		nullptr
	);

	m_memory = m_context->getHeapGarlic()->alloc(
		sizeAndAlign.m_size,
		sizeAndAlign.m_align,
		true
	);
	if (!m_memory)
		return false;

	m_renderTarget.setAddresses(
		m_memory->getPointer(),
		nullptr,
		nullptr
	);

	m_width = setDesc.width;
	m_height = setDesc.height;

	return true;
}

void RenderTargetPs4::destroy()
{
	if (m_memory)
	{
		m_memory->free();
		m_memory = 0;
	}
}

ITexture* RenderTargetPs4::resolve()
{
	return this;
}

int RenderTargetPs4::getWidth() const
{
	return m_width;
}

int RenderTargetPs4::getHeight() const
{
	return m_height;
}

bool RenderTargetPs4::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetPs4::unlock(int level)
{
}

	}
}
