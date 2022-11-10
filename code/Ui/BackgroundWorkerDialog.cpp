/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/IWaitable.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/Static.h"
#include "Ui/StyleSheet.h"
#include "Ui/TableLayout.h"
#include "Ui/BackgroundWorkerDialog.h"
#include "Ui/ProgressBar.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.BackgroundWorkerDialog", BackgroundWorkerDialog, Dialog)

bool BackgroundWorkerDialog::create(ui::Widget* parent, const std::wstring& title, const std::wstring& message, bool abortButton)
{
	if (!ui::Dialog::create(parent, title, dpi96(380), dpi96(150), WsCenterParent, new ui::TableLayout(L"100%", L"*,*,*", dpi96(4), dpi96(4))))
		return false;

	addEventHandler< TimerEvent >(this, &BackgroundWorkerDialog::eventTimer);
	addEventHandler< PaintEvent >(this, &BackgroundWorkerDialog::eventPaint);

	m_labelMessage = new Static();
	m_labelMessage->create(this, message);

	m_labelStatus = new Static();
	m_labelStatus->create(this, L"...");

	m_progressBar = new ProgressBar();
	m_progressBar->create(this);

	if (abortButton)
	{
		m_buttonAbort = new Button();
		m_buttonAbort->create(this, L"Abort");
		m_buttonAbort->addEventHandler< ButtonClickEvent >(this, &BackgroundWorkerDialog::eventAbortClick);
	}

	fit(Container::FaVertical);

	startTimer(100);
	return true;
}

bool BackgroundWorkerDialog::execute(IWaitable* waitable, IWorkerStatus* status)
{
	m_status = status;
	m_waitables.push_back(waitable);

	showModal();

	m_status = nullptr;
	m_waitables.resize(0);
	return true;
}

bool BackgroundWorkerDialog::execute(const std::vector< IWaitable* >& waitables, IWorkerStatus* status)
{
	m_status = status;
	m_waitables = waitables;

	showModal();

	m_status = nullptr;
	m_waitables.resize(0);
	return true;
}

void BackgroundWorkerDialog::eventAbortClick(ButtonClickEvent* event)
{
	m_buttonAbort->setEnable(false);

	stopTimer();
	endModal(DialogResult::Cancel);
}

void BackgroundWorkerDialog::eventTimer(TimerEvent* event)
{
	// Check if we're are finished.
	bool finished = true;
	for (auto waitable : m_waitables)
	{
		if (!waitable->wait(0))
		{
			finished = false;
			break;
		}
	}

	// Update progression status.
	if (m_status)
	{
		std::wstring status = L"";
		int32_t step = -1;

		if (m_status->read(step, status))
		{
			m_labelStatus->setText(status);

			if (step >= 0)
			{
				m_progressBar->setRange(0, 1000);
				m_progressBar->setProgress(step);
			}
			else
				m_progressBar->setRange(0, 0);

			update(nullptr, true);
		}
	}
	else
	{
		// No status callback; just keep updating progress bar.
		update(nullptr, true);
	}

	// End dialog if all job threads are finished.
	if (finished)
		endModal(DialogResult::Ok);
}

void BackgroundWorkerDialog::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = Widget::getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	canvas.setForeground(ss->getColor(this, L"border-color"));
	canvas.drawRect(rcInner);

	event->consume();
}

	}
}
