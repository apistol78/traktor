/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Acquire.h"
#include "Render/Types.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/RenderTargetDepthDx11.h"
#include "Render/Dx11/RenderTargetDx11.h"
#include "Render/Dx11/RenderTargetSetDx11.h"
#include "Render/Dx11/TypesDx11.h"
#include "Render/Dx11/Utilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetDx11", RenderTargetSetDx11, RenderTargetSet)

RenderTargetSetDx11::RenderTargetSetDx11(ContextDx11* context)
:	RenderTargetSet()
,	m_context(context)
,	m_width(0)
,	m_height(0)
,	m_usingPrimaryDepthStencil(false)
{
}

RenderTargetSetDx11::~RenderTargetSetDx11()
{
	destroy();
}

bool RenderTargetSetDx11::create(const RenderTargetSetCreateDesc& setDesc)
{
	m_colorTextures.resize(setDesc.count);
	for (int i = 0; i < setDesc.count; ++i)
	{
		m_colorTextures[i] = new RenderTargetDx11(m_context);
		if (!m_colorTextures[i]->create(setDesc, setDesc.targets[i]))
			return false;
	}

	if (setDesc.createDepthStencil)
	{
		m_depthTexture = new RenderTargetDepthDx11(m_context);
		if (!m_depthTexture->create(setDesc))
			return false;
	}

	m_width = setDesc.width;
	m_height = setDesc.height;
	m_usingPrimaryDepthStencil = setDesc.usingPrimaryDepthStencil;

	return true;
}

void RenderTargetSetDx11::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	for (RefArray< RenderTargetDx11 >::iterator i = m_colorTextures.begin(); i != m_colorTextures.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_colorTextures.resize(0);
	safeDestroy(m_depthTexture);
}

int RenderTargetSetDx11::getWidth() const
{
	return m_width;
}

int RenderTargetSetDx11::getHeight() const
{
	return m_height;
}

ISimpleTexture* RenderTargetSetDx11::getColorTexture(int index) const
{
	return index < int(m_colorTextures.size()) ? m_colorTextures[index] : 0;
}

ISimpleTexture* RenderTargetSetDx11::getDepthTexture() const
{
	return m_depthTexture;
}

void RenderTargetSetDx11::swap(int index1, int index2)
{
	std::swap(m_colorTextures[index1], m_colorTextures[index2]);
}

void RenderTargetSetDx11::discard()
{
}

bool RenderTargetSetDx11::read(int index, void* buffer) const
{
	return false;
}

	}
}
