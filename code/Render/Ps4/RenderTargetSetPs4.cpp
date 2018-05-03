/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Render/Types.h"
#include "Render/Ps4/ContextPs4.h"
#include "Render/Ps4/RenderTargetDepthPs4.h"
#include "Render/Ps4/RenderTargetPs4.h"
#include "Render/Ps4/RenderTargetSetPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetPs4", RenderTargetSetPs4, RenderTargetSet)

RenderTargetSetPs4::RenderTargetSetPs4(ContextPs4* context)
:	RenderTargetSet()
,	m_context(context)
{
}

RenderTargetSetPs4::~RenderTargetSetPs4()
{
	destroy();
}

bool RenderTargetSetPs4::create(const RenderTargetSetCreateDesc& setDesc)
{
	m_colorTargets.resize(setDesc.count);
	for (int i = 0; i < setDesc.count; ++i)
	{
		m_colorTargets[i] = new RenderTargetPs4(m_context);
		if (!m_colorTargets[i]->create(setDesc, setDesc.targets[i]))
			return false;
	}

	if (setDesc.createDepthStencil)
	{
		m_depthTarget = new RenderTargetDepthPs4(m_context);
		if (!m_depthTarget->create(setDesc))
			return false;
	}

	return true;
}

void RenderTargetSetPs4::destroy()
{
	for (RefArray< RenderTargetPs4 >::iterator i = m_colorTargets.begin(); i != m_colorTargets.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_colorTargets.resize(0);
	safeDestroy(m_depthTarget);
}

int RenderTargetSetPs4::getWidth() const
{
	return 0;
}

int RenderTargetSetPs4::getHeight() const
{
	return 0;
}

ISimpleTexture* RenderTargetSetPs4::getColorTexture(int index) const
{
	return index < int(m_colorTargets.size()) ? m_colorTargets[index] : 0;
}

ISimpleTexture* RenderTargetSetPs4::getDepthTexture() const
{
	return m_depthTarget;
}

void RenderTargetSetPs4::swap(int index1, int index2)
{
	//std::swap(m_colorTextures[index1], m_colorTextures[index2]);
}

void RenderTargetSetPs4::discard()
{
}

bool RenderTargetSetPs4::isContentValid() const
{
	return true;
}

bool RenderTargetSetPs4::read(int index, void* buffer) const
{
	return false;
}

	}
}
