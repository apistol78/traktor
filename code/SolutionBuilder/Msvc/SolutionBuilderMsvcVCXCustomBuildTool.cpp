/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/OutputStream.h"
#include "Core/Io/Path.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/System/ResolveEnv.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXCustomBuildTool.h"
#include "SolutionBuilder/Msvc/GeneratorContext.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcVCXCustomBuildTool", 0, SolutionBuilderMsvcVCXCustomBuildTool, SolutionBuilderMsvcVCXBuildTool)

bool SolutionBuilderMsvcVCXCustomBuildTool::generateProject(
	GeneratorContext& context,
	const Solution* solution,
	const Project* project,
	const std::wstring& filter,
	const Path& fileName,
	OutputStream& os
) const
{
	if (compareIgnoreCase(m_fileType, fileName.getExtension()) != 0)
		return true;

	os << L"<" << m_name << L" Include=\"" << fileName.getPathName() << L"\">" << Endl;
	os << IncreaseIndent;

	os << L"<Command>" << resolveEnv(context.format(m_command), nullptr) << L"</Command>" << Endl;
	os << L"<Message>" << context.format(m_message) << L"</Message>" << Endl;
	os << L"<Outputs>" << resolveEnv(context.format(m_outputs), nullptr) << L"</Outputs>" << Endl;

	os << DecreaseIndent;
	os << L"</" << m_name << L">" << Endl;

	return true;
}

void SolutionBuilderMsvcVCXCustomBuildTool::serialize(ISerializer& s)
{
	SolutionBuilderMsvcVCXBuildTool::serialize(s);
	s >> Member< std::wstring >(L"command", m_command);
	s >> Member< std::wstring >(L"message", m_message);
	s >> Member< std::wstring >(L"outputs", m_outputs);
}

}
