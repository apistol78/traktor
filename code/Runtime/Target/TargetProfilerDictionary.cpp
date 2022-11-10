/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Target/TargetProfilerDictionary.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberSmallMap.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TargetProfilerDictionary", 0, TargetProfilerDictionary, ISerializable)

TargetProfilerDictionary::TargetProfilerDictionary(const SmallMap< uint16_t, std::wstring >& dictionary)
:	m_dictionary(dictionary)
{
}

void TargetProfilerDictionary::serialize(ISerializer& s)
{
	s >> MemberSmallMap< uint16_t, std::wstring >(L"dictionary", m_dictionary);
}

	}
}
