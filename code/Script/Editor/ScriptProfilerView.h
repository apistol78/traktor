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

private:
	struct ProfileEntry
	{
		double inclusiveDuration;
		double exclusiveDuration;
		uint64_t count;
		Ref< ui::custom::GridRow > row;

		ProfileEntry()
		:	inclusiveDuration(0.0)
		,	exclusiveDuration(0.0)
		,	count(0)
		{
		}
	};

	Ref< IScriptProfiler > m_scriptProfiler;
	Ref< ui::custom::GridView > m_profileGrid;
	std::map< std::wstring, ProfileEntry > m_profile;

	/*! \name IScriptProfiler::IListener */
	/*! \{ */

	virtual void callMeasured(const std::wstring& function, double timeStamp, double inclusiveDuration, double exclusiveDuration);

	/*! \} */
};

	}
}

#endif	// traktor_script_ScriptProfilerView_H
