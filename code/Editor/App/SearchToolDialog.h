/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Dialog.h"

namespace traktor
{

class Job;

	namespace ui
	{

class Button;
class CheckBox;
class Edit;
class GridView;
class ProgressBar;

	}

	namespace editor
	{

class IEditor;

class SearchToolDialog : public ui::Dialog
{
	T_RTTI_CLASS;

public:
	SearchToolDialog(IEditor* editor);

	virtual void destroy() override final;

	bool create(ui::Widget* parent);

	virtual void show() override final;

private:
	IEditor* m_editor;
	Ref< ui::Edit > m_editSearch;
	Ref< ui::CheckBox > m_checkRegExp;
	Ref< ui::CheckBox > m_checkCaseSensitive;
	Ref< ui::Button > m_buttonFind;
	Ref< ui::Button > m_buttonSaveAs;
	Ref< ui::ProgressBar > m_progressBar;
	Ref< ui::GridView > m_gridResults;
	Ref< Job > m_jobSearch;

	void search(const std::wstring& needle);

	void eventSearchKey(ui::KeyDownEvent* event);

	void eventButtonSearchClick(ui::ButtonClickEvent* event);

	void eventButtonSaveAsClick(ui::ButtonClickEvent* event);

	void eventGridResultDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventGridResultButtonUp(ui::MouseButtonUpEvent* event);

	void eventTimer(ui::TimerEvent* event);

	void eventClose(ui::CloseEvent* event);

	void jobSearch(std::wstring needle, bool regExp, bool caseSensitive);
};

	}
}

