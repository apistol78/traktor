/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Render/SH/SHMatrix.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SHMatrix", 0, SHMatrix, ISerializable)

SHMatrix::SHMatrix(int32_t rows, int32_t columns)
:	m_rows(rows)
,	m_columns(columns)
,	m_elements(rows * columns)
{
}

void SHMatrix::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"rows", m_rows);
	s >> Member< int32_t >(L"columns", m_columns);
	s >> MemberAlignedVector< float >(L"elements", m_elements);
}

}
