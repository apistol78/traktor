/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXClCompileBuildTool.h"

#include "Core/Io/OutputStream.h"
#include "Core/Io/Path.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "SolutionBuilder/Msvc/GeneratorContext.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sb.SolutionBuilderMsvcVCXClCompileBuildTool", 0, SolutionBuilderMsvcVCXClCompileBuildTool, SolutionBuilderMsvcVCXBuildTool)

bool SolutionBuilderMsvcVCXClCompileBuildTool::generateProject(
	GeneratorContext& context,
	const Solution* solution,
	const Project* project,
	const std::wstring& filter,
	const Path& fileName,
	OutputStream& os) const
{
	if (compareIgnoreCase(m_fileType, fileName.getExtension()) != 0)
		return true;

	if (!filter.empty())
	{
		os << L"<" << m_name << L" Include=\"" << fileName.getPathName() << L"\">" << Endl;
		os << L"\t<ObjectFileName>$(IntDir)</ObjectFileName>" << Endl;
		os << L"</" << m_name << L">" << Endl;
	}
	else
		os << L"<" << m_name << L" Include=\"" << fileName.getPathName() << L"\" />" << Endl;

	return true;
}

}
