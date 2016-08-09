#include "Render/Types.h"
#include "Render/Vulkan/RenderTargetVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetVk", RenderTargetVk, ISimpleTexture)

RenderTargetVk::RenderTargetVk()
:	m_width(0)
,	m_height(0)
{
}

RenderTargetVk::~RenderTargetVk()
{
	destroy();
}

bool RenderTargetVk::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc)
{
	m_width = setDesc.width;
	m_height = setDesc.height;
	return true;
}

void RenderTargetVk::destroy()
{
}

ITexture* RenderTargetVk::resolve()
{
	return this;
}

int RenderTargetVk::getWidth() const
{
	return m_width;
}

int RenderTargetVk::getHeight() const
{
	return m_height;
}

bool RenderTargetVk::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetVk::unlock(int level)
{
}

	}
}
