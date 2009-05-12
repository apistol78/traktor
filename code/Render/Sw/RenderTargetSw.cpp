#include "Render/Sw/RenderTargetSw.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSw", RenderTargetSw, Texture)

RenderTargetSw::RenderTargetSw()
:	m_width(0)
,	m_height(0)
{
}

bool RenderTargetSw::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc)
{
	if (desc.format >= TfR16G16B16A16F)
		return false;
	if (setDesc.width <= 0 || setDesc.height <= 0)
		return false;

	m_width = setDesc.width;
	m_height = setDesc.height;

	m_color.resize(setDesc.width * setDesc.height);
	return true;
}

void RenderTargetSw::destroy()
{
	m_width =
	m_height = 0;
	m_color.resize(0);
}

int RenderTargetSw::getWidth() const
{
	return m_width;
}

int RenderTargetSw::getHeight() const
{
	return m_height;
}

int RenderTargetSw::getDepth() const
{
	return 1;
}

uint32_t* RenderTargetSw::getColorSurface()
{
	return &m_color[0];
}

	}
}
