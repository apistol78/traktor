#include "Core/System/OS.h"
#include "Runtime/Editor/LaunchRenderDocTool.h"
#include "Ui/StyleBitmap.h"

namespace traktor
{
	namespace runtime
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
		return false;

#if defined(_WIN32)
	std::wstring renderDoc = renderDocPath.getPathName();
#else
	std::wstring renderDoc = renderDocPath.getPathNameNoVolume();
#endif
	std::wstring commandLine = L"\"" + renderDoc + L"\"";

	return OS::getInstance().execute(
		commandLine,
		L"",
		nullptr,
		OS::EfMute) != nullptr;
}

	}
}
