#include "Core/Class/BoxedAllocator.h"
#include "Render/Frame/RenderPass.h"

namespace traktor::render
{
	namespace
	{

BoxedAllocator< RenderPass, 128 > s_allocRenderPass;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderPass", RenderPass, Object)

RenderPass::RenderPass(const std::wstring& name)
:   m_name(name)
{
	m_output.resourceId = ~0;
}

void RenderPass::setName(const std::wstring& name)
{
	m_name = name;
}

void RenderPass::addInput(handle_t resourceId)
{
	// Just ignore invalid target set id; convenient when setting up passes to be able to add null inputs.
	if (resourceId == 0)
		return;

	auto& input = m_inputs.push_back();
	input.resourceId = resourceId;
}

bool RenderPass::requireInput(handle_t resourceId) const
{
	for (const auto& input : m_inputs)
	{
		if (input.resourceId == resourceId)
			return true;
	}
	return false;
}

void RenderPass::setOutput(handle_t resourceId)
{
	m_output.resourceId = resourceId;
	m_output.clear.mask = 0;
	m_output.load = 0;
	m_output.store = 0;
}

void RenderPass::setOutput(handle_t resourceId, uint32_t load, uint32_t store)
{
	m_output.resourceId = resourceId;
	m_output.clear.mask = 0;
	m_output.load = load;
	m_output.store = store;
}

void RenderPass::setOutput(handle_t resourceId, const Clear& clear, uint32_t load, uint32_t store)
{
	m_output.resourceId = resourceId;
	m_output.clear = clear;
	m_output.load = load;
	m_output.store = store;
}

void RenderPass::addBuild(const fn_build_t& build)
{
	m_builds.push_back(build);
}

void* RenderPass::operator new (size_t size)
{
	return s_allocRenderPass.alloc();
}

void RenderPass::operator delete (void* ptr)
{
	s_allocRenderPass.free(ptr);
}

}
