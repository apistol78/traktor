#ifndef traktor_script_ScriptProfilerView_H
#define traktor_script_ScriptProfilerView_H

#include "Script/IScriptProfiler.h"
#include "Ui/Container.h"

namespace traktor
{
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
	ScriptProfilerView(IScriptProfiler* scriptProfiler);

	virtual ~ScriptProfilerView();

	bool create(ui::Widget* parent);

	void destroy();

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

	Ref< IScriptProfiler > m_scriptProfiler;
	Ref< ui::custom::ToolBar > m_profilerTools;
	Ref< ui::custom::GridView > m_profileGrid;
	std::map< std::wstring, ProfileEntry > m_profile;

	void updateProfileGrid();

	void eventProfilerToolClick(ui::custom::ToolBarButtonClickEvent* event);

	/*! \name IScriptProfiler::IListener */
	/*! \{ */

	virtual void callMeasured(const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration);

	/*! \} */
};

	}
}

#endif	// traktor_script_ScriptProfilerView_H
