#include "Core/Misc/SafeDestroy.h"
#include "Render/Types.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetVk", RenderTargetSetVk, RenderTargetSet)

RenderTargetSetVk::RenderTargetSetVk()
:	RenderTargetSet()
{
}

RenderTargetSetVk::~RenderTargetSetVk()
{
	destroy();
}

bool RenderTargetSetVk::create(const RenderTargetSetCreateDesc& setDesc)
{
	m_colorTargets.resize(setDesc.count);
	for (int i = 0; i < setDesc.count; ++i)
	{
		m_colorTargets[i] = new RenderTargetVk();
		if (!m_colorTargets[i]->create(setDesc, setDesc.targets[i]))
			return false;
	}

	if (setDesc.createDepthStencil)
	{
		m_depthTarget = new RenderTargetDepthVk();
		if (!m_depthTarget->create(setDesc))
			return false;
	}

	return true;
}

void RenderTargetSetVk::destroy()
{
	for (RefArray< RenderTargetVk >::iterator i = m_colorTargets.begin(); i != m_colorTargets.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_colorTargets.resize(0);
	safeDestroy(m_depthTarget);
}

int RenderTargetSetVk::getWidth() const
{
	return 0;
}

int RenderTargetSetVk::getHeight() const
{
	return 0;
}

ISimpleTexture* RenderTargetSetVk::getColorTexture(int index) const
{
	return index < int(m_colorTargets.size()) ? m_colorTargets[index] : 0;
}

ISimpleTexture* RenderTargetSetVk::getDepthTexture() const
{
	return m_depthTarget;
}

void RenderTargetSetVk::swap(int index1, int index2)
{
	//std::swap(m_colorTextures[index1], m_colorTextures[index2]);
}

void RenderTargetSetVk::discard()
{
}

bool RenderTargetSetVk::read(int index, void* buffer) const
{
	return false;
}

	}
}
