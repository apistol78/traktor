/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Script/IScriptProfiler.h"
#include "Ui/Container.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::ui
{

class Command;
class GridRow;
class GridView;
class ToolBar;
class ToolBarButtonClickEvent;

}

namespace traktor::script
{

class ScriptProfilerView
:	public ui::Container
,	public IScriptProfiler::IListener
{
	T_RTTI_CLASS;

public:
	explicit ScriptProfilerView(editor::IEditor* editor, IScriptProfiler* scriptProfiler);

	virtual ~ScriptProfilerView();

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

	bool handleCommand(const ui::Command& command);

private:
	struct ProfileEntry
	{
		uint64_t callCount;
		double inclusiveDuration;
		double exclusiveDuration;
		Ref< ui::GridRow > row;

		ProfileEntry()
		:	callCount(0)
		,	inclusiveDuration(0.0)
		,	exclusiveDuration(0.0)
		{
		}
	};

	editor::IEditor* m_editor;
	Ref< IScriptProfiler > m_scriptProfiler;
	Ref< ui::ToolBar > m_profilerTools;
	Ref< ui::GridView > m_profileGrid;
	std::map< std::pair< Guid, std::wstring >, ProfileEntry > m_profile;

	void updateProfileGrid();

	void eventProfilerToolClick(ui::ToolBarButtonClickEvent* event);

	void eventProfileGridDoubleClick(ui::MouseDoubleClickEvent* event);

	/*! \name IScriptProfiler::IListener */
	/*! \{ */

	virtual void callEnter(const Guid& scriptId, const std::wstring& function) override final;

	virtual void callLeave(const Guid& scriptId, const std::wstring& function) override final;

	virtual void callMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration) override final;

	/*! \} */
};

}
