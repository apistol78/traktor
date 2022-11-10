/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Chunks of memory.
 * \ingroup Core
 */
class T_DLLCLASS ChunkMemory : public Object
{
	T_RTTI_CLASS;

public:
	struct Chunk
	{
		void* head;
		void* ptr;
		size_t size;
		size_t avail;
	};

	virtual ~ChunkMemory();

	/*! Get byte size of chunked memory. */
	size_t size() const;

	/*! Resize chunked memory. */
	bool resize(size_t size);

	/*! Get chunk memory. */
	Chunk getChunk(size_t position) const;

private:
	AlignedVector< uint8_t* > m_chunks;
	int64_t m_size = 0;
};

}