/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Render/IAccelerationStructure.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class ApiBuffer;
class Context;

class AccelerationStructureVk : public IAccelerationStructure
{
	T_RTTI_CLASS;

public:
	explicit AccelerationStructureVk(Context* context, ApiBuffer* buffer, VkAccelerationStructureKHR as);

	virtual ~AccelerationStructureVk();

protected:
	Context* m_context = nullptr;
	Ref< ApiBuffer > m_buffer;
	VkAccelerationStructureKHR m_as;
};

}
