/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor::render
{

class ApiBuffer;
class Context;
class UniformBufferChain;

/*!
 * \ingroup Render
 */
struct UniformBufferRange
{
	UniformBufferChain* chain = nullptr;
	uint32_t offset = 0;
	void* ptr = nullptr;
};

/*! Chain of equally sized uniform buffer blocks.
 * \ingroup Render
 */
class UniformBufferChain : public Object
{
public:
	static Ref< UniformBufferChain > create(Context* context, uint32_t blockCount, uint32_t blockSize);

	void destroy();

	bool allocate(UniformBufferRange& outRange);

	void free(const UniformBufferRange& range);

	ApiBuffer* getBuffer() const { return m_buffer; }

private:
	Ref< ApiBuffer > m_buffer;
	uint8_t* m_top = nullptr;
	uint32_t m_blockSize = 0;
	AlignedVector< uint32_t > m_free;	//!< Indices of free blocks, last is allocated first.

	explicit UniformBufferChain(ApiBuffer* buffer, void* top, uint32_t blockCount, uint32_t blockSize);
};

}
