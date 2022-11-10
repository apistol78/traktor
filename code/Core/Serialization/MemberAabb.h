/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Aabb2.h"
#include "Core/Math/Aabb3.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

class MemberAabb2 : public MemberComplex
{
public:
	explicit MemberAabb2(const wchar_t* const name, Aabb2& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< Vector2 >(L"mn", m_ref.mn);
		s >> Member< Vector2 >(L"mx", m_ref.mx);
	}

private:
	Aabb2& m_ref;
};

class MemberAabb3 : public MemberComplex
{
public:
	explicit MemberAabb3(const wchar_t* const name, Aabb3& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< Vector4 >(L"mn", m_ref.mn, AttributePoint());
		s >> Member< Vector4 >(L"mx", m_ref.mx, AttributePoint());
	}

private:
	Aabb3& m_ref;
};

}

