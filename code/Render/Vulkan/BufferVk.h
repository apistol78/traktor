/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Buffer.h"

namespace traktor::render
{

class Context;

class BufferVk : public Buffer
{
	T_RTTI_CLASS;

public:
	explicit BufferVk(Context* context, uint32_t bufferSize, uint32_t& instances);

	virtual ~BufferVk();

protected:
	Context* m_context = nullptr;
	uint32_t& m_instances;
};

}
