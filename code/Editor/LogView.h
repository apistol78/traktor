/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Container.h"
#include "Ui/LogList/LogList.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ILogTarget;

}

namespace traktor::ui
{

class Edit;
class LogActivateEvent;
class Menu;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;

}

namespace traktor::editor
{

class IEditor;

class T_DLLCLASS LogView
:	public ui::Container
,	public ui::LogList::ISymbolLookup
{
	T_RTTI_CLASS;

public:
	explicit LogView(IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

	ILogTarget* getLogTarget() const { return m_logTarget; }

private:
	IEditor* m_editor;
	Ref< ui::ToolBarButton > m_toolToggleInfo;
	Ref< ui::ToolBarButton > m_toolToggleWarning;
	Ref< ui::ToolBarButton > m_toolToggleError;
	Ref< ui::ToolBar > m_toolFilter;
	Ref< ui::Edit > m_editFind;
	Ref< ui::LogList > m_log;
	Ref< ui::Menu > m_popup;
	Ref< ILogTarget > m_logTarget;

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventFindKey(ui::KeyEvent* event);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventLogActivate(ui::LogActivateEvent* event);

	virtual bool lookupLogSymbol(const Guid& symbolId, std::wstring& outSymbol) const override final;
};

}
