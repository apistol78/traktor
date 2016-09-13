#include "Core/Misc/SafeDestroy.h"
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
	T_FATAL_ASSERT_M (m_renderTargetSet, L"Render error: Render target set already destroyed.");
	safeDestroy(m_renderTargetSet);
}

int RenderTargetSetCapture::getWidth() const
{
	T_FATAL_ASSERT_M (m_renderTargetSet, L"Render error: Render target set destroyed.");
	return m_renderTargetSet->getWidth();
}
	
int RenderTargetSetCapture::getHeight() const
{
	T_FATAL_ASSERT_M (m_renderTargetSet, L"Render error: Render target set destroyed.");
	return m_renderTargetSet->getHeight();
}

ISimpleTexture* RenderTargetSetCapture::getColorTexture(int index) const
{
	T_FATAL_ASSERT_M (m_renderTargetSet, L"Render error: Render target set destroyed.");
	return new SimpleTextureCapture(m_renderTargetSet->getColorTexture(index));
}

ISimpleTexture* RenderTargetSetCapture::getDepthTexture() const
{
	T_FATAL_ASSERT_M (m_renderTargetSet, L"Render error: Render target set destroyed.");
	return new SimpleTextureCapture(m_renderTargetSet->getDepthTexture());
}

void RenderTargetSetCapture::swap(int index1, int index2)
{
	T_FATAL_ASSERT_M (m_renderTargetSet, L"Render error: Render target set destroyed.");
	m_renderTargetSet->swap(index1, index2);
}

void RenderTargetSetCapture::discard()
{
	T_FATAL_ASSERT_M (m_renderTargetSet, L"Render error: Render target set destroyed.");
	m_renderTargetSet->discard();
}

bool RenderTargetSetCapture::read(int index, void* buffer) const
{
	T_FATAL_ASSERT_M (m_renderTargetSet, L"Render error: Render target set destroyed.");
	T_FATAL_ASSERT_M (index >= 0, L"Render error: Incorrect read-back index.");
	return m_renderTargetSet->read(index, buffer);
}

	}
}
