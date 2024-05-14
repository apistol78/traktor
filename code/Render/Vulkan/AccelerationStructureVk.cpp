/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/AccelerationStructureVk.h"
#include "Render/Vulkan/Private/ApiBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/ApiLoader.h"

namespace traktor::render
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.AccelerationStructureVk", AccelerationStructureVk, IAccelerationStructure)

AccelerationStructureVk::AccelerationStructureVk(Context* context, ApiBuffer* buffer, VkAccelerationStructureKHR as)
:	m_context(context)
,	m_buffer(buffer)
,	m_as(as)
{
}

AccelerationStructureVk::~AccelerationStructureVk()
{
	if (m_context != nullptr)
	{
		m_context->addDeferredCleanup([
			as = m_as
		](Context* cx) {
			vkDestroyAccelerationStructureKHR(cx->getLogicalDevice(), as, nullptr);
		});
	}
	safeDestroy(m_buffer);
	m_context = nullptr;
}

}
