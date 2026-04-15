/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/Editor/EmptyMovieAsset.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.EmptyMovieAsset", 0, EmptyMovieAsset, ISerializable)

void EmptyMovieAsset::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"stageWidth", m_stageWidth);
	s >> Member< int32_t >(L"stageHeight", m_stageHeight);
	s >> Member< int32_t >(L"frameRate", m_frameRate);
	s >> Member< Color4ub >(L"backgroundColor", m_backgroundColor);
}

}
