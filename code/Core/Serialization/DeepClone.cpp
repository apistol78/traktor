/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Io/ChunkMemoryStream.h"

namespace traktor
{
	namespace
	{

const uint32_t c_initialCapacity = 4096;	//!< Estimated initial size of clone; used to reduce number of allocation of m_copy array.

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.DeepClone", DeepClone, Object)

DeepClone::DeepClone(const ISerializable* source)
{
	ChunkMemoryStream stream(&m_memory, false, true);
	BinarySerializer(&stream).writeObject(source);
}

Ref< ISerializable > DeepClone::create() const
{
	ChunkMemoryStream stream(&m_memory, true, false);
	return BinarySerializer(&stream).readObject();
}

}
