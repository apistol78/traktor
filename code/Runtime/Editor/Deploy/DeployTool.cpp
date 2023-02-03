/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/Deploy/DeployTool.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.DeployTool", 0, DeployTool, ISerializable)

const std::wstring& DeployTool::getExecutable() const
{
	return m_executable;
}

const std::map< std::wstring, std::wstring >& DeployTool::getEnvironment() const
{
	return m_environment;
}

void DeployTool::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"executable", m_executable);
	s >> MemberStlMap< std::wstring, std::wstring >(L"environment", m_environment);
}

}
