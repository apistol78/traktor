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
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Spark/Movie.h"
#include "Spark/Editor/MovieAsset.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.MovieAsset", 2, MovieAsset, editor::Asset)

void MovieAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	s >> Member< bool >(L"staticMovie", m_staticMovie);

	if (s.getVersion< MovieAsset >() < 1)
	{
		bool includeAS;
		s >> Member< bool >(L"includeAS", includeAS);
	}

	if (s.getVersion< MovieAsset >() >= 2)
		s >> MemberAlignedVector< Font, MemberComposite< Font > >(L"fonts", m_fonts);
}

void MovieAsset::Font::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Path >(L"fileName", fileName);
}

}
