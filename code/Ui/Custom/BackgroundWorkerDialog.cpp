#include "Ui/Custom/BackgroundWorkerDialog.h"
#include "Ui/Custom/BackgroundWorkerStatus.h"
#include "Ui/Custom/ProgressBar.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Static.h"
#include "Ui/Button.h"
#include "Core/Thread/IWaitable.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.BackgroundWorkerDialog", BackgroundWorkerDialog, Dialog)

BackgroundWorkerDialog::BackgroundWorkerDialog()
{
}

bool BackgroundWorkerDialog::create(ui::Widget* parent, const std::wstring& title, const std::wstring& message, int style)
{
	if (!ui::Dialog::create(parent, title, 300, 150, style, new ui::TableLayout(L"300", L"*,*,*", 4, 4)))
		return false;

	addTimerEventHandler(createMethodHandler(this, &BackgroundWorkerDialog::eventTimer));

	m_labelMessage = new Static();
	m_labelMessage->create(this, message);

	m_labelStatus = new Static();
	m_labelStatus->create(this, L"...");

	m_progressBar = new ProgressBar();
	m_progressBar->create(this);

	if (style & WsAbortButton)
	{
		m_buttonAbort = new Button();
		m_buttonAbort->create(this, L"Abort");
		m_buttonAbort->addClickEventHandler(createMethodHandler(this, &BackgroundWorkerDialog::eventAbortClick));
	}

	fit();

	startTimer(100);
	return true;
}

bool BackgroundWorkerDialog::execute(IWaitable* waitable, BackgroundWorkerStatus* status)
{
	m_status = status;

	if (m_status)
		m_progressBar->setRange(0, m_status->getSteps());
	else
		m_progressBar->setRange(0, 0);

	m_waitables.push_back(waitable);

	showModal();
	
	m_status = 0;
	m_waitables.resize(0);
	return true;
}

bool BackgroundWorkerDialog::execute(const std::vector< IWaitable* >& waitables, BackgroundWorkerStatus* status)
{
	m_status = status;

	if (m_status)
		m_progressBar->setRange(0, m_status->getSteps());
	else
		m_progressBar->setRange(0, 0);

	m_waitables = waitables;

	showModal();

	m_status = 0;
	m_waitables.resize(0);
	return true;
}

void BackgroundWorkerDialog::eventAbortClick(Event* event)
{
	m_buttonAbort->setEnable(false);
	
	stopTimer();
	endModal(DrCancel);
}

void BackgroundWorkerDialog::eventTimer(Event* event)
{
	// Check if we're are finished.
	bool finished = true;
	for (std::vector< IWaitable* >::const_iterator i = m_waitables.begin(); i != m_waitables.end(); ++i)
	{
		if (!(*i)->wait(0))
		{
			finished = false;
			break;
		}
	}

	// Update progression status.
	if (m_status)
	{
		BackgroundWorkerStatus::Notification notification;
		if (m_status->readNotification(notification))
		{
			m_labelStatus->setText(notification.status);
			if (m_status->getSteps() > 0)
				m_progressBar->setProgress(notification.step);
			update(0, true);
		}
	}
	else
	{
		// No status callback; just keep updating progress bar.
		update(0, true);
	}

	// End dialog if all job threads are finished.
	if (finished)
		endModal(DrOk);
}

		}
	}
}
