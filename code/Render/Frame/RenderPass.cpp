/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Frame/RenderPass.h"

#include "Core/Class/BoxedAllocator.h"

namespace traktor::render
{
namespace
{

BoxedAllocator< RenderPass, 256 > s_allocRenderPass;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderPass", RenderPass, Object)

RenderPass::RenderPass(const std::wstring& name)
	: m_name(name)
{
}

void RenderPass::setName(const std::wstring& name)
{
	m_name = name;
}

void RenderPass::addInput(handle_t resourceId)
{
	auto& input = m_inputs.push_back();
	input.resourceId = resourceId;
}

void RenderPass::addInput(RGTargetSet targetSet)
{
	// Just ignore invalid target set id; convenient when setting up passes to be able to add null inputs.
	if (targetSet == RGTargetSet::Invalid || targetSet == RGTargetSet::Output)
		return;

	auto& input = m_inputs.push_back();
	input.resourceId = targetSet.get();
}

void RenderPass::addInput(RGBuffer buffer)
{
	// Just ignore invalid buffer id; convenient when setting up passes to be able to add null inputs.
	if (buffer == RGBuffer::Invalid)
		return;

	auto& input = m_inputs.push_back();
	input.resourceId = buffer.get();
}

void RenderPass::addWeakInput(handle_t resourceId)
{
	// #todo Currently we can get away with not doing anything but
	// we need to revisit this to ensure resource life time.
}

void RenderPass::setOutput(handle_t resourceId)
{
	m_output.resourceId = resourceId;
	m_output.clear.mask = 0;
	m_output.load = 0;
	m_output.store = 0;
}

void RenderPass::setOutput(RGTargetSet targetId, uint32_t load, uint32_t store)
{
	m_output.resourceId = targetId.get();
	m_output.clear.mask = 0;
	m_output.load = load;
	m_output.store = store;
}

void RenderPass::setOutput(RGTargetSet targetId, const Clear& clear, uint32_t load, uint32_t store)
{
	m_output.resourceId = targetId.get();
	m_output.clear = clear;
	m_output.load = load;
	m_output.store = store;
}

void RenderPass::setOutput(RGBuffer bufferId)
{
	m_output.resourceId = bufferId.get();
	m_output.clear.mask = 0;
	m_output.load = 0;
	m_output.store = 0;
}

void RenderPass::addBuild(const fn_build_t& build)
{
	m_builds.push_back(build);
}

void* RenderPass::operator new(size_t size)
{
	return s_allocRenderPass.alloc();
}

void RenderPass::operator delete(void* ptr)
{
	s_allocRenderPass.free(ptr);
}

}
