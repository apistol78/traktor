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
#include "Core/Serialization/MemberComplex.h"
#include "Physics/DofJointDesc.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

class MemberLimit : public MemberComplex
{
public:
	MemberLimit(const wchar_t* const name, DofJointDesc::Limit& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< bool >(L"x", m_ref.x);
		s >> Member< bool >(L"y", m_ref.y);
		s >> Member< bool >(L"z", m_ref.z);
	}

private:
	DofJointDesc::Limit& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.DofJointDesc", 0, DofJointDesc, JointDesc)

void DofJointDesc::serialize(ISerializer& s)
{
	s >> MemberLimit(L"translate", m_translate);
	s >> MemberLimit(L"rotate", m_rotate);
}

	}
}
