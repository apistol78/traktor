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
#include "Core/Serialization/MemberEnum.h"
#include "Database/Local/Perforce/PerforceClientDesc.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.PerforceClientDesc", 0, PerforceClientDesc, ISerializable)

void PerforceClientDesc::serialize(ISerializer& s)
{
	const MemberEnum< SecurityLevel >::Key c_SecurityLevel_Keys[] =
	{
		{ L"SlLow", SlLow },
		{ L"SlHigh", SlHigh },
		0
	};

	s >> Member< std::wstring >(L"host", m_host);
	s >> Member< std::wstring >(L"port", m_port);
	s >> Member< std::wstring >(L"user", m_user);
	s >> Member< std::wstring >(L"password", m_password);
	s >> Member< std::wstring >(L"client", m_client);
	s >> MemberEnum< SecurityLevel >(L"securityLevel", m_securityLevel, c_SecurityLevel_Keys);
}

	}
}
