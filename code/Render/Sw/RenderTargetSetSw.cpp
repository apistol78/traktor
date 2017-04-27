/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/Align.h"
#include "Render/Types.h"
#include "Render/Sw/RenderTargetSetSw.h"
#include "Render/Sw/RenderTargetSw.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetSw", RenderTargetSetSw, RenderTargetSet)

RenderTargetSetSw::RenderTargetSetSw()
:	m_width(0)
,	m_height(0)
,	m_usingPrimaryDepth(false)
{
}

bool RenderTargetSetSw::create(const RenderTargetSetCreateDesc& desc)
{
	if (desc.width <= 0 || desc.height <= 0)
		return false;

	m_width = desc.width;
	m_height = desc.height;
	m_usingPrimaryDepth = desc.usingPrimaryDepthStencil;

	m_colorTargets.resize(desc.count);
	for (int i = 0; i < desc.count; ++i)
	{
		m_colorTargets[i] = new RenderTargetSw();
		if (!m_colorTargets[i]->create(desc, desc.targets[i]))
			return false;
	}

	if (desc.createDepthStencil)
	{
		if (m_usingPrimaryDepth)
			return false;

		m_depthSurface.reset((float*)Alloc::acquireAlign(
			alignUp(desc.width * desc.height, 4) * sizeof(float),
			16,
			T_FILE_LINE
		));

		m_stencilSurface.reset((uint8_t*)Alloc::acquireAlign(
			alignUp(desc.width * desc.height, 4) * sizeof(uint8_t),
			16,
			T_FILE_LINE
		));
	}

	return true;
}

void RenderTargetSetSw::destroy()
{
	m_width =
	m_height = 0;
	m_usingPrimaryDepth = false;

	for (RefArray< RenderTargetSw >::iterator i = m_colorTargets.begin(); i != m_colorTargets.end(); ++i)
		(*i)->destroy();

	m_colorTargets.resize(0);
	m_depthSurface.release();
	m_stencilSurface.release();
}

int RenderTargetSetSw::getWidth() const
{
	return m_width;
}

int RenderTargetSetSw::getHeight() const
{
	return m_height;
}

ISimpleTexture* RenderTargetSetSw::getColorTexture(int index) const
{
	return m_colorTargets[index];
}

ISimpleTexture* RenderTargetSetSw::getDepthTexture() const
{
	return 0;
}

void RenderTargetSetSw::swap(int index1, int index2)
{
	std::swap(m_colorTargets[index1], m_colorTargets[index2]);
}

void RenderTargetSetSw::discard()
{
}

bool RenderTargetSetSw::read(int index, void* buffer) const
{
	return false;
}

	}
}
