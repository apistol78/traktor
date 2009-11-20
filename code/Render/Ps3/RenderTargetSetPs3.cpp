#include "Render/Ps3/RenderTargetSetPs3.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetPs3", RenderTargetSetPs3, RenderTargetSet)

RenderTargetPs3::RenderTargetPs3()
{
}

bool RenderTargetPs3::create(const RenderTargetSetCreateDesc& desc)
{
	return true;
}

void RenderTargetPs3::destroy()
{
}

int RenderTargetPs3::getWidth() const
{
	return m_width;
}

int RenderTargetPs3::getHeight() const
{
	return m_height;
}

Ref< ITexture > RenderTargetPs3::getColorTexture(int index) const
{
}

void RenderTargetPs3::swap(int index1, int index2)
{
}

bool RenderTargetPs3::read(int index, void* buffer) const
{
	return false;
}

	}
}
