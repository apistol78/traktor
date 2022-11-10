/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Container.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class IBitmap;

/*! Top level form.
 * \ingroup UI
 *
 * Form is a top level widget, i.e. the application
 * window.
 */
class T_DLLCLASS Form : public Container
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsDefault = WsResizable | WsSystemBox | WsMinimizeBox | WsMaximizeBox | WsCloseBox | WsCaption
	};

	enum NotifyStyle
	{
		NsApplication = 1,
		NsSystemTray = 2
	};

	bool create(const std::wstring& text, int width, int height, int style = WsDefault, Layout* layout = 0, Widget* parent = 0);

	void setIcon(IBitmap* icon);

	IBitmap* getIcon();

	void maximize();

	void minimize();

	void restore();

	bool isMaximized() const;

	bool isMinimized() const;

	void hideProgress();

	void showProgress(int32_t current, int32_t total);

	virtual bool acceptLayout() const override;

private:
	Ref< IBitmap > m_icon;
};

	}
}

