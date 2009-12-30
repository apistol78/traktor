#include "Core/Log/Log.h"
#include "Render/Ps3/RenderTargetSetPs3.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/LocalMemoryManager.h"
#include "Render/Ps3/LocalMemoryObject.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetPs3", RenderTargetSetPs3, RenderTargetSet)

RenderTargetSetPs3::RenderTargetSetPs3()
:	m_width(0)
,	m_height(0)
,	m_depthData(0)
{
	std::memset(&m_depthTexture, 0, sizeof(m_depthTexture));
}

bool RenderTargetSetPs3::create(const RenderTargetSetCreateDesc& desc)
{
	m_width = desc.width;
	m_height = desc.height;

	m_renderTargets.resize(desc.count);
	for (int32_t i = 0; i < desc.count; ++i)
	{
		m_renderTargets[i] = new RenderTargetPs3();
		if (!m_renderTargets[i]->create(desc, desc.targets[i]))
			return false;
	}

	if (desc.depthStencil)
	{
		m_depthTexture.format = CELL_GCM_TEXTURE_DEPTH24_D8 | CELL_GCM_TEXTURE_LN;
		m_depthTexture.mipmap = 1;
		m_depthTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
		m_depthTexture.cubemap = 0;
		m_depthTexture.remap = 0;
		m_depthTexture.width = m_width;
		m_depthTexture.height = m_height;
		m_depthTexture.depth = 1;
		m_depthTexture.location = CELL_GCM_LOCATION_LOCAL;
		m_depthTexture.pitch = m_width * 4;
		m_depthTexture.offset = 0;

		uint32_t depthSize = m_depthTexture.pitch * m_depthTexture.height;
		m_depthData = LocalMemoryManager::getInstance().alloc(depthSize, 4096, false);
	}
	else
	{
		m_depthTexture.width = 0;
		m_depthTexture.height = 0;
		m_depthTexture.depth = 0;
		m_depthTexture.location = CELL_GCM_LOCATION_LOCAL;
		m_depthTexture.pitch = 64;
		m_depthTexture.offset = 0;
	}

	return true;
}

void RenderTargetSetPs3::destroy()
{
}

int RenderTargetSetPs3::getWidth() const
{
	return m_width;
}

int RenderTargetSetPs3::getHeight() const
{
	return m_height;
}

Ref< ITexture > RenderTargetSetPs3::getColorTexture(int index) const
{
	return m_renderTargets[index];
}

void RenderTargetSetPs3::swap(int index1, int index2)
{
	Ref< RenderTargetPs3 > rt1 = m_renderTargets[index1];
	Ref< RenderTargetPs3 > rt2 = m_renderTargets[index2];
	m_renderTargets[index1] = rt2;
	m_renderTargets[index2] = rt1;
}

bool RenderTargetSetPs3::read(int index, void* buffer) const
{
	return false;
}

const CellGcmTexture& RenderTargetSetPs3::getGcmDepthTexture()
{
	if (m_depthData)
		m_depthTexture.offset = m_depthData->getOffset();

	return m_depthTexture;
}

	}
}
