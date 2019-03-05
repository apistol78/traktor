#include "Runtime/Editor/LaunchPs3TargetManagerTool.h"
#include "Core/System/OS.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.LaunchPs3TargetManagerTool", 0, LaunchPs3TargetManagerTool, IEditorTool)

std::wstring LaunchPs3TargetManagerTool::getDescription() const
{
	return L"Launch PS3 TargetManager...";
}

Ref< ui::IBitmap > LaunchPs3TargetManagerTool::getIcon() const
{
	return nullptr;
}

bool LaunchPs3TargetManagerTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool LaunchPs3TargetManagerTool::launch(ui::Widget* parent, editor::IEditor* runtime, const PropertyGroup* param)
{
	return OS::getInstance().execute(L"\"$(SN_PS3_PATH)/bin/ps3tm.exe\"", L"$(SN_PS3_PATH)/bin", 0, false, false, false) != 0;
}

	}
}
