/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Engine/LayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.LayerData", LayerData, ISerializable)

void LayerData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< bool >(L"permitTransition", m_permitTransition);
}

	}
}
