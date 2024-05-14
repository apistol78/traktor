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
#include "Ui/Unit.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class ChildEvent;
class IBitmap;

/*! Dialog
 * \ingroup UI
 */
class T_DLLCLASS Dialog : public Container
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t WsCenterParent = WsUser;
	constexpr static uint32_t WsCenterDesktop = (WsUser << 1);
	constexpr static uint32_t WsDefaultFixed = WsSystemBox | WsMinimizeBox | WsCloseBox | WsCaption;
	constexpr static uint32_t WsDefaultResizable = WsResizable | WsSystemBox | WsMinimizeBox | WsMaximizeBox | WsCloseBox | WsCaption;

	bool create(Widget* parent, const std::wstring& text, Unit width, Unit height, uint32_t style = WsDefaultResizable, Layout* layout = 0);

	void setIcon(IBitmap* icon);

	virtual DialogResult showModal();

	virtual void endModal(DialogResult result);

	bool isModal() const;

	virtual bool isEnable(bool includingParents) const override;

	virtual bool acceptLayout() const override;

private:
	bool m_modal = false;

	void eventChild(ChildEvent* event);
};

}
