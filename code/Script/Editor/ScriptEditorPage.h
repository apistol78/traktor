/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Editor/IEditorPage.h"
#include "Script/IErrorCallback.h"
#include "Script/IScriptDebugger.h"
#include "Script/Editor/IScriptDebuggerSessions.h"
#include "Script/Editor/IScriptOutline.h"
#include "Ui/SyntaxRichEdit/SyntaxTypes.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Preprocessor;

}

namespace traktor::editor
{

class IDocument;
class IEditor;
class IEditorPageSite;

}

namespace traktor::ui
{

class IBitmap;
class GridRow;
class GridView;
class Menu;
class Splitter;
class SyntaxRichEdit;
class StatusBar;
class Tab;
class ToolBar;
class ToolBarButtonClickEvent;

}

namespace traktor::script
{

class IScriptCompiler;
class IScriptContext;
class Script;
class ScriptBreakpointEvent;
class ScriptClassesView;

class T_DLLCLASS ScriptEditorPage
:	public editor::IEditorPage
,	public IErrorCallback
,	public IScriptDebugger::IListener
,	public IScriptDebuggerSessions::IListener
{
	T_RTTI_CLASS;

public:
	explicit ScriptEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;

	std::wstring m_assetPath;
	Ref< Script > m_script;
	Ref< IScriptDebuggerSessions > m_scriptDebuggerSessions;
	Ref< IScriptCompiler > m_scriptCompiler;
	Ref< IScriptOutline > m_scriptOutline;
	Ref< Preprocessor > m_preprocessor;
	Ref< ui::IBitmap > m_bitmapFunction;
	Ref< ui::IBitmap > m_bitmapFunctionLocal;
	Ref< ui::IBitmap > m_bitmapFunctionReference;
	Ref< ui::Container > m_containerExplorer;
	Ref< ui::GridView > m_outlineGrid;
	Ref< ScriptClassesView > m_classesView;
	Ref< ui::SyntaxRichEdit > m_edit;
	Ref< ui::Menu > m_editMenu;
	Ref< ui::StatusBar > m_compileStatus;
	int32_t m_compileCountDown;
	int32_t m_debugBreadcrumbAttribute;
	int32_t m_debugCurrentAttribute;

	/*! \name IErrorCallback */
	/*! \{ */

	virtual void syntaxError(const std::wstring& name, uint32_t line, const std::wstring& message) override final;

	virtual void otherError(const std::wstring& message) override final;

	/*! \} */

	/*! \name IScriptDebugger::IListener */
	/*! \{ */

	virtual void debugeeStateChange(IScriptDebugger* scriptDebugger) override final;

	/*! \} */

	/*! \name IScriptDebuggerSessions::IListener */
	/*! \{ */

	virtual void notifyBeginSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) override final;

	virtual void notifyEndSession(IScriptDebugger* scriptDebugger, IScriptProfiler* scriptProfiler) override final;

	virtual void notifySetBreakpoint(const Guid& scriptId, int32_t lineNumber) override final;

	virtual void notifyRemoveBreakpoint(const Guid& scriptId, int32_t lineNumber) override final;

	/*! \} */

	void updateBreakpoints();

	void buildOutlineGrid(ui::GridView* grid, ui::GridRow* parent, const IScriptOutline::Node* on);

	void eventOutlineDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventScriptChange(ui::ContentChangeEvent* event);

	void eventScriptButtonDown(ui::MouseButtonDownEvent* event);

	void eventScriptButtonUp(ui::MouseButtonUpEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

}
