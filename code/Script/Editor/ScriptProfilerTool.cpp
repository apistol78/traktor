#include "Core/Misc/SafeDestroy.h"
#include "Script/Editor/ScriptProfilerTool.h"
#include "Script/Editor/ScriptProfilerDialog.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace script
	{


T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptProfilerTool", 0, ScriptProfilerTool, IEditorTool)

std::wstring ScriptProfilerTool::getDescription() const
{
	return i18n::Text(L"SCRIPT_PROFILER_TOOL");
}

bool ScriptProfilerTool::launch(ui::Widget* parent, editor::IEditor* script)
{
	safeDestroy(m_profilerDialog);

	m_profilerDialog = new ScriptProfilerDialog(script);
	if (m_profilerDialog->create(parent))
		m_profilerDialog->show();
	else
		safeDestroy(m_profilerDialog);

	return true;
}

	}
}
