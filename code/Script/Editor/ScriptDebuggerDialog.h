/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Script/Editor/IScriptDebuggerSessions.h"
#include "Ui/Dialog.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::ui
{

class Tab;

}

namespace traktor::script
{

class IScriptDebuggerSessions;

class ScriptDebuggerDialog
	: public ui::Dialog
	, public IScriptDebuggerSessions::IListener
{
	T_RTTI_CLASS;

public:
	explicit ScriptDebuggerDialog(editor::IEditor* editor);

	virtual void destroy() override final;

	bool create(ui::Widget* parent);

private:
	editor::IEditor* m_editor;
	Ref< IScriptDebuggerSessions > m_scriptDebuggerSessions;
	Ref< ui::Tab > m_tabSessions;

	/*! \name IScriptDebuggerSessions::IListener */
	/*! \{ */

	virtual void notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) override final;

	virtual void notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) override final;

	virtual void notifySetBreakpoint(const std::wstring& fileName, int32_t lineNumber) override final;

	virtual void notifyRemoveBreakpoint(const std::wstring& fileName, int32_t lineNumber) override final;

	/*! \} */
};

}
