/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/System/OS.h"
#include "Runtime/Editor/LaunchRenderDocTool.h"
#include "Ui/StyleBitmap.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.LaunchRenderDocTool", 0, LaunchRenderDocTool, IEditorTool)

std::wstring LaunchRenderDocTool::getDescription() const
{
	return L"Launch RenderDoc...";
}

Ref< ui::IBitmap > LaunchRenderDocTool::getIcon() const
{
	return new ui::StyleBitmap(L"Runtime.RenderDoc");
}

bool LaunchRenderDocTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool LaunchRenderDocTool::launch(ui::Widget* parent, editor::IEditor* runtime, const PropertyGroup* param)
{
	Path renderDocPath;
	if (!OS::getInstance().whereIs(L"renderdocui", renderDocPath))
	{
#if defined(_WIN32)
		// Get path to installed RenderDoc.
		std::wstring iconPath;
		if (!OS::getInstance().getRegistry(L"HKEY_LOCAL_MACHINE", L"SOFTWARE\\Classes\\RenderDoc.RDCCapture.1\\DefaultIcon", L"", iconPath))
			return false;

		renderDocPath = Path(iconPath).getPathOnlyOS() + L"/renderdocui";
#else
		return false;
#endif
	}

	const std::wstring renderDoc = renderDocPath.getPathName();
	const std::wstring commandLine = L"\"" + renderDoc + L"\"";

	return OS::getInstance().execute(
		commandLine,
		L"",
		nullptr,
		OS::EfMute) != nullptr;
}

}
