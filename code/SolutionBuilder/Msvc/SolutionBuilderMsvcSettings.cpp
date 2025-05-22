/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcSettings.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXProj.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sb.SolutionBuilderMsvcSettings", 0, SolutionBuilderMsvcSettings, ISerializable)

void SolutionBuilderMsvcSettings::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"slnVersion", m_slnVersion);
	s >> Member< std::wstring >(L"vsVersion", m_vsVersion);
	s >> MemberRef< SolutionBuilderMsvcVCXProj >(L"project", m_project);
}

}
