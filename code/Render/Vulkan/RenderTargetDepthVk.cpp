#include "Render/Types.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDepthVk", RenderTargetDepthVk, ISimpleTexture)

RenderTargetDepthVk::RenderTargetDepthVk()
:	m_width(0)
,	m_height(0)
{
}

RenderTargetDepthVk::~RenderTargetDepthVk()
{
	destroy();
}

bool RenderTargetDepthVk::create(const RenderTargetSetCreateDesc& setDesc)
{
	m_width = setDesc.width;
	m_height = setDesc.height;
	return true;
}

void RenderTargetDepthVk::destroy()
{
}

ITexture* RenderTargetDepthVk::resolve()
{
	return this;
}

int RenderTargetDepthVk::getWidth() const
{
	return m_width;
}

int RenderTargetDepthVk::getHeight() const
{
	return m_height;
}

bool RenderTargetDepthVk::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetDepthVk::unlock(int level)
{
}

void* RenderTargetDepthVk::getInternalHandle()
{
	return 0;
}

	}
}
