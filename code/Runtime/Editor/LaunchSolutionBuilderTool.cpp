/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/System/OS.h"
#include "Editor/IEditor.h"
#include "Runtime/Editor/LaunchSolutionBuilderTool.h"
#include "Ui/StyleBitmap.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.LaunchSolutionBuilderTool", 0, LaunchSolutionBuilderTool, IEditorTool)

std::wstring LaunchSolutionBuilderTool::getDescription() const
{
	return L"Launch SolutionBuilder...";
}

Ref< ui::IBitmap > LaunchSolutionBuilderTool::getIcon() const
{
	return nullptr;
}

bool LaunchSolutionBuilderTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool LaunchSolutionBuilderTool::launch(ui::Widget* parent, editor::IEditor* editor, const PropertyGroup* param)
{
	const std::wstring styleSheet = editor->getSettings()->getProperty< std::wstring >(L"Editor.StyleSheet", L"$(TRAKTOR_HOME)/resources/runtime/themes/Light/StyleSheet.xss");
	return OS::getInstance().execute(
#if defined(_WIN32)
		std::wstring(L"$(TRAKTOR_HOME)/bin/latest/win64/releaseshared/Traktor.SolutionBuilder.Editor.App.exe -styleSheet=\"") + styleSheet + L"\"",
#else
		std::wstring(L"$(TRAKTOR_HOME)/bin/latest/linux/releaseshared/Traktor.SolutionBuilder.Editor.App -styleSheet=\"") + styleSheet + L"\"",
#endif
		L"",
		nullptr,
		OS::EfMute) != nullptr;
}

}
