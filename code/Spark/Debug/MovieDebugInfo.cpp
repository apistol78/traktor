/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/Debug/MovieDebugInfo.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.MovieDebugInfo", 0, MovieDebugInfo, ISerializable)

MovieDebugInfo::MovieDebugInfo()
{
}

MovieDebugInfo::MovieDebugInfo(const std::wstring& name)
:	m_name(name)
{
}

void MovieDebugInfo::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
}

	}
}
