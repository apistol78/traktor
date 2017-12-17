/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Render/Capture/Error.h"
#include "Render/Capture/RenderTargetSetCapture.h"
#include "Render/Capture/SimpleTextureCapture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetCapture", RenderTargetSetCapture, RenderTargetSet)

RenderTargetSetCapture::RenderTargetSetCapture(RenderTargetSet* renderTargetSet)
:	m_renderTargetSet(renderTargetSet)
{
}

void RenderTargetSetCapture::destroy()
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set already destroyed.");
	
	m_colorTextures[0].reset();
	m_colorTextures[1].reset();
	m_colorTextures[2].reset();
	m_colorTextures[3].reset();
	m_depthTexture.reset();
	
	safeDestroy(m_renderTargetSet);
}

int RenderTargetSetCapture::getWidth() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->getWidth() : 0;
}
	
int RenderTargetSetCapture::getHeight() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->getHeight() : 0;
}

ISimpleTexture* RenderTargetSetCapture::getColorTexture(int index) const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");

	if (!m_renderTargetSet)
		return 0;

	if (!m_colorTextures[index])
	{
		Ref< ISimpleTexture > colorTexture = m_renderTargetSet->getColorTexture(index);
		if (!colorTexture)
			return 0;

		m_colorTextures[index] = new SimpleTextureCapture(colorTexture);
	}

	return m_colorTextures[index];
}

ISimpleTexture* RenderTargetSetCapture::getDepthTexture() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");

	if (!m_renderTargetSet)
		return 0;

	if (!m_depthTexture)
	{
		Ref< ISimpleTexture > depthTexture = m_renderTargetSet->getDepthTexture();
		if (!depthTexture)
			return 0;

		m_depthTexture = new SimpleTextureCapture(depthTexture);
	}

	return m_depthTexture;
}

void RenderTargetSetCapture::swap(int index1, int index2)
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");

	if (!m_renderTargetSet)
		return;

	T_CAPTURE_ASSERT (index1 >= 0, L"Swap index 1 invalid");
	T_CAPTURE_ASSERT (index2 >= 0, L"Swap index 2 invalid");
	T_CAPTURE_ASSERT (index1 < 4, L"Swap index 1 invalid");
	T_CAPTURE_ASSERT (index2 < 4, L"Swap index 2 invalid");

	m_renderTargetSet->swap(index1, index2);

	Ref< SimpleTextureCapture > tmp = m_colorTextures[index1];
	m_colorTextures[index1] = m_colorTextures[index2];
	m_colorTextures[index2] = tmp;
}

void RenderTargetSetCapture::discard()
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	if (m_renderTargetSet)
		m_renderTargetSet->discard();
}

bool RenderTargetSetCapture::isContentValid() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->isContentValid() : false;
}

bool RenderTargetSetCapture::read(int index, void* buffer) const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	T_CAPTURE_ASSERT (index >= 0, L"Incorrect read-back index.");
	return m_renderTargetSet ? m_renderTargetSet->read(index, buffer) : false;
}

	}
}
