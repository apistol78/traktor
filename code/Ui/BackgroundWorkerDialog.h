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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IWaitable;

}

namespace traktor::ui
{

class Button;
class Static;
class ProgressBar;

/*! Background worker thread progress dialog.
 * \ingroup UI
 */
class T_DLLCLASS BackgroundWorkerDialog : public Dialog
{
	T_RTTI_CLASS;

public:
	struct IWorkerStatus : public IRefCount
	{
		virtual bool read(int32_t& outStep, std::wstring& outStatus) = 0;
	};

	bool create(Widget* parent, const std::wstring& title, const std::wstring& message, bool abortButton);

	bool execute(IWaitable* waitable, IWorkerStatus* status);

	bool execute(const std::vector< IWaitable* >& waitables, IWorkerStatus* status);

private:
	std::vector< IWaitable* > m_waitables;
	Ref< IWorkerStatus > m_status;
	Ref< Static > m_labelMessage;
	Ref< Static > m_labelStatus;
	Ref< ProgressBar > m_progressBar;
	Ref< Button > m_buttonAbort;

	void eventAbortClick(ButtonClickEvent* event);

	void eventTimer(TimerEvent* event);

	void eventPaint(PaintEvent* event);
};

}
