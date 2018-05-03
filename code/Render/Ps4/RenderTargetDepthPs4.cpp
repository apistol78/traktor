/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Types.h"
#include "Render/Ps4/ContextPs4.h"
#include "Render/Ps4/MemoryHeapPs4.h"
#include "Render/Ps4/MemoryHeapObjectPs4.h"
#include "Render/Ps4/RenderTargetDepthPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDepthPs4", RenderTargetDepthPs4, ISimpleTexture)

RenderTargetDepthPs4::RenderTargetDepthPs4(ContextPs4* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
{
}

RenderTargetDepthPs4::~RenderTargetDepthPs4()
{
	destroy();
}

bool RenderTargetDepthPs4::create(const RenderTargetSetCreateDesc& setDesc)
{
	static const sce::Gnm::ZFormat kZFormat = sce::Gnm::kZFormat32Float;
	static const sce::Gnm::StencilFormat kStencilFormat = sce::Gnm::kStencil8;
	static const bool kHtileEnabled = true;

	sce::Gnm::DataFormat depthFormat = sce::Gnm::DataFormat::build(kZFormat);

	sce::Gnm::TileMode depthTileMode;
	sce::GpuAddress::computeSurfaceTileMode(
		&depthTileMode,
		sce::GpuAddress::kSurfaceTypeDepthOnlyTarget,
		depthFormat,
		1
	);

	sce::Gnm::SizeAlign stencilSizeAndAlign;
	sce::Gnm::SizeAlign htileSizeAndAlign;

	const sce::Gnm::SizeAlign sizeAndAlign = m_depthTarget.init(
		setDesc.width,
		setDesc.height,
		depthFormat.getZFormat(),
		kStencilFormat,
		depthTileMode,
		sce::Gnm::kNumFragments1,
		kStencilFormat != sce::Gnm::kStencilInvalid ? &stencilSizeAndAlign : nullptr,
		kHtileEnabled ? &htileSizeAndAlign : nullptr
	);

	if (kHtileEnabled)
	{
		m_memoryHTile = m_context->getHeapGarlic()->alloc(
			htileSizeAndAlign.m_size,
			htileSizeAndAlign.m_align,
			true
		);
		if (!m_memoryHTile)
			return false;

		m_depthTarget.setHtileAddress(m_memoryHTile->getPointer());
	}

	if (kStencilFormat != sce::Gnm::kStencilInvalid)
	{
		m_memoryStencil = m_context->getHeapGarlic()->alloc(
			stencilSizeAndAlign.m_size,
			stencilSizeAndAlign.m_align,
			true
		);
	}

	m_memory = m_context->getHeapGarlic()->alloc(
		sizeAndAlign.m_size,
		sizeAndAlign.m_align,
		true
	);
	if (!m_memory)
		return false;

	m_depthTarget.setAddresses(
		m_memory->getPointer(),
		m_memoryStencil ? m_memoryStencil->getPointer() : nullptr
	);

	m_width = setDesc.width;
	m_height = setDesc.height;

	return true;
}

void RenderTargetDepthPs4::destroy()
{
	if (m_memory)
	{
		m_memory->free();
		m_memory = 0;
	}
}

ITexture* RenderTargetDepthPs4::resolve()
{
	return this;
}

int RenderTargetDepthPs4::getWidth() const
{
	return m_width;
}

int RenderTargetDepthPs4::getHeight() const
{
	return m_height;
}

bool RenderTargetDepthPs4::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetDepthPs4::unlock(int level)
{
}

void* RenderTargetDepthPs4::getInternalHandle()
{
	return 0;
}

	}
}
