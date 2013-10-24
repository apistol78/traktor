#include "Render/Sw/RenderTargetSw.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSw", RenderTargetSw, ISimpleTexture)

RenderTargetSw::RenderTargetSw()
:	m_width(0)
,	m_height(0)
{
}

bool RenderTargetSw::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc)
{
	if (desc.format >= TfDXT1)
		return false;
	if (setDesc.width <= 0 || setDesc.height <= 0)
		return false;

	m_width = setDesc.width;
	m_height = setDesc.height;

	m_color.reset(new uint16_t [setDesc.width * setDesc.height]);
	return true;
}

void RenderTargetSw::destroy()
{
	m_width =
	m_height = 0;
	m_color.release();
}

ITexture* RenderTargetSw::resolve()
{
	return this;
}

int RenderTargetSw::getWidth() const
{
	return m_width;
}

int RenderTargetSw::getHeight() const
{
	return m_height;
}

bool RenderTargetSw::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetSw::unlock(int level)
{
}

uint16_t* RenderTargetSw::getColorSurface()
{
	return m_color.ptr();
}

	}
}
