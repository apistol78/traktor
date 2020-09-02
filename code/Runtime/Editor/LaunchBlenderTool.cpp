#include "Core/System/OS.h"
#include "Runtime/Editor/LaunchBlenderTool.h"
#include "Ui/StyleBitmap.h"

namespace traktor
{
	namespace runtime
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

#if defined(_WIN32)
	std::wstring blender = blenderPath.getPathName();
#else
	std::wstring blender = blenderPath.getPathNameNoVolume();
#endif
	std::wstring commandLine = L"\"" + blender + L"\"";

	return OS::getInstance().execute(
		commandLine,
		L"",
		nullptr,
		OS::EfMute) != nullptr;
}

	}
}
