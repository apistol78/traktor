#include "Core/Misc/SafeDestroy.h"
#include "Render/Capture/Error.h"
#include "Render/Capture/RenderTargetSetCapture.h"
#include "Render/Capture/SimpleTextureCapture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetCapture", RenderTargetSetCapture, IRenderTargetSet)

RenderTargetSetCapture::RenderTargetSetCapture(IRenderTargetSet* renderTargetSet)
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

int32_t RenderTargetSetCapture::getWidth() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->getWidth() : 0;
}

int32_t RenderTargetSetCapture::getHeight() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->getHeight() : 0;
}

ISimpleTexture* RenderTargetSetCapture::getColorTexture(int32_t index) const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");

	if (!m_renderTargetSet)
		return nullptr;

	if (!m_colorTextures[index])
	{
		Ref< ISimpleTexture > colorTexture = m_renderTargetSet->getColorTexture(index);
		if (!colorTexture)
			return nullptr;

		m_colorTextures[index] = new SimpleTextureCapture(colorTexture);
	}

	return m_colorTextures[index];
}

ISimpleTexture* RenderTargetSetCapture::getDepthTexture() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");

	if (!m_renderTargetSet)
		return nullptr;

	if (!m_depthTexture)
	{
		Ref< ISimpleTexture > depthTexture = m_renderTargetSet->getDepthTexture();
		if (!depthTexture)
			return nullptr;

		m_depthTexture = new SimpleTextureCapture(depthTexture);
	}

	return m_depthTexture;
}

bool RenderTargetSetCapture::isContentValid() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->isContentValid() : false;
}

bool RenderTargetSetCapture::read(int32_t index, void* buffer) const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	T_CAPTURE_ASSERT (index >= 0, L"Incorrect read-back index.");
	return m_renderTargetSet ? m_renderTargetSet->read(index, buffer) : false;
}

bool RenderTargetSetCapture::haveColorTexture(int32_t index) const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->getColorTexture(index) != 0 : false;
}

bool RenderTargetSetCapture::haveDepthTexture() const
{
	T_CAPTURE_ASSERT (m_renderTargetSet, L"Render target set destroyed.");
	return m_renderTargetSet ? m_renderTargetSet->getDepthTexture() != 0 : false;
}

	}
}
