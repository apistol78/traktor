#include "Render/Sw/RenderTargetSetSw.h"
#include "Render/Sw/RenderTargetSw.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetSw", RenderTargetSetSw, RenderTargetSet)

RenderTargetSetSw::RenderTargetSetSw()
:	m_width(0)
,	m_height(0)
{
}

bool RenderTargetSetSw::create(const RenderTargetSetCreateDesc& desc)
{
	if (desc.width <= 0 || desc.height <= 0)
		return false;

	m_width = desc.width;
	m_height = desc.height;

	m_colorTargets.resize(desc.count);
	for (int i = 0; i < desc.count; ++i)
	{
		m_colorTargets[i] = new RenderTargetSw();
		if (!m_colorTargets[i]->create(desc, desc.targets[i]))
			return false;
	}

	if (desc.createDepthStencil)
		m_depthSurface.reset(new uint16_t [desc.width * desc.height]);

	return true;
}

void RenderTargetSetSw::destroy()
{
	m_width =
	m_height = 0;

	for (RefArray< RenderTargetSw >::iterator i = m_colorTargets.begin(); i != m_colorTargets.end(); ++i)
		(*i)->destroy();

	m_colorTargets.resize(0);
	m_depthSurface.release();
}

int RenderTargetSetSw::getWidth() const
{
	return m_width;
}

int RenderTargetSetSw::getHeight() const
{
	return m_height;
}

Ref< ISimpleTexture > RenderTargetSetSw::getColorTexture(int index) const
{
	return m_colorTargets[index];
}

void RenderTargetSetSw::swap(int index1, int index2)
{
	std::swap(m_colorTargets[index1], m_colorTargets[index2]);
}

bool RenderTargetSetSw::read(int index, void* buffer) const
{
	return false;
}

uint16_t* RenderTargetSetSw::getDepthSurface()
{
	return m_depthSurface.ptr();
}

	}
}
