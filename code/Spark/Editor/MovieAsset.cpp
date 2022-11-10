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
#include "Spark/Movie.h"
#include "Spark/Editor/MovieAsset.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.MovieAsset", 1, MovieAsset, editor::Asset)

MovieAsset::MovieAsset()
:	m_staticMovie(false)
{
}

void MovieAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	s >> Member< bool >(L"staticMovie", m_staticMovie);

	if (s.getVersion< MovieAsset >() < 1)
	{
		bool includeAS;
		s >> Member< bool >(L"includeAS", includeAS);
	}
}

	}
}
