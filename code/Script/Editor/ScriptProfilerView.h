#ifndef traktor_script_ScriptProfilerView_H
#define traktor_script_ScriptProfilerView_H

#include "Script/IScriptProfiler.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Command;

		namespace custom
		{

class GridRow;
class GridView;
class ToolBar;
class ToolBarButtonClickEvent;

		}
	}

	namespace script
	{

class ScriptProfilerView
:	public ui::Container
,	public IScriptProfiler::IListener
{
	T_RTTI_CLASS;

public:
	ScriptProfilerView(editor::IEditor* editor, IScriptProfiler* scriptProfiler);

	virtual ~ScriptProfilerView();

	bool create(ui::Widget* parent);

	virtual void destroy() T_OVERRIDE T_FINAL;

	bool handleCommand(const ui::Command& command);

private:
	struct ProfileEntry
	{
		uint64_t callCount;
		double inclusiveDuration;
		double exclusiveDuration;
		Ref< ui::custom::GridRow > row;

		ProfileEntry()
		:	callCount(0)
		,	inclusiveDuration(0.0)
		,	exclusiveDuration(0.0)
		{
		}
	};

	editor::IEditor* m_editor;
	Ref< IScriptProfiler > m_scriptProfiler;
	Ref< ui::custom::ToolBar > m_profilerTools;
	Ref< ui::custom::GridView > m_profileGrid;
	std::map< std::pair< Guid, std::wstring >, ProfileEntry > m_profile;

	void updateProfileGrid();

	void eventProfilerToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventProfileGridDoubleClick(ui::MouseDoubleClickEvent* event);

	/*! \name IScriptProfiler::IListener */
	/*! \{ */

	virtual void callEnter(const Guid& scriptId, const std::wstring& function) T_OVERRIDE T_FINAL;

	virtual void callLeave(const Guid& scriptId, const std::wstring& function) T_OVERRIDE T_FINAL;

	virtual void callMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration) T_OVERRIDE T_FINAL;

	/*! \} */
};

	}
}

#endif	// traktor_script_ScriptProfilerView_H
