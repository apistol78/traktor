/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/System/OS.h"
#include "Runtime/Editor/LaunchBlenderTool.h"
#include "Ui/StyleBitmap.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.LaunchBlenderTool", 0, LaunchBlenderTool, IEditorTool)

std::wstring LaunchBlenderTool::getDescription() const
{
	return L"Launch Blender...";
}

Ref< ui::IBitmap > LaunchBlenderTool::getIcon() const
{
	return new ui::StyleBitmap(L"Runtime.Blender");
}

bool LaunchBlenderTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool LaunchBlenderTool::launch(ui::Widget* parent, editor::IEditor* runtime, const PropertyGroup* param)
{
	Path blenderPath;
	if (!OS::getInstance().whereIs(L"blender", blenderPath))
		return false;

	const std::wstring blender = blenderPath.getPathNameOS();
	const std::wstring commandLine = L"\"" + blender + L"\"";

	return OS::getInstance().execute(
		commandLine,
		L"",
		nullptr,
		OS::EfDetach) != nullptr;
}

}
