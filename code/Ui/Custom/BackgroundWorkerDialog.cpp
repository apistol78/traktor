#include "Ui/Custom/BackgroundWorkerDialog.h"
#include "Ui/Custom/BackgroundWorkerStatus.h"
#include "Ui/Custom/ProgressBar.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Static.h"
#include "Ui/Button.h"
#include "Core/Thread/Thread.h"

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
	if (!ui::Dialog::create(parent, title, 300, 150, style, gc_new< ui::TableLayout >(L"300", L"*,*,*", 4, 4)))
		return false;

	addTimerEventHandler(createMethodHandler(this, &BackgroundWorkerDialog::eventTimer));

	m_labelMessage = gc_new< Static >();
	m_labelMessage->create(this, message);

	m_labelStatus = gc_new< Static >();
	m_labelStatus->create(this, L"...");

	m_progressBar = gc_new< ProgressBar >();
	m_progressBar->create(this);

	if (style & WsAbortButton)
	{
		m_buttonAbort = gc_new< Button >();
		m_buttonAbort->create(this, L"Abort");
		m_buttonAbort->addClickEventHandler(createMethodHandler(this, &BackgroundWorkerDialog::eventAbortClick));
	}

	fit();

	startTimer(100);
	return true;
}

bool BackgroundWorkerDialog::execute(Thread* thread, BackgroundWorkerStatus* status)
{
	m_status = status;

	if (m_status)
		m_progressBar->setRange(0, m_status->getSteps());

	m_threads.push_back(thread);
	m_threads.front()->start();

	showModal();
	
	m_status = 0;
	m_threads.resize(0);
	return true;
}

bool BackgroundWorkerDialog::execute(const std::vector< Thread* >& threads, BackgroundWorkerStatus* status)
{
	m_status = status;

	if (m_status)
		m_progressBar->setRange(0, m_status->getSteps());

	m_threads = threads;
	for (std::vector< Thread* >::const_iterator i = m_threads.begin(); i != m_threads.end(); ++i)
		(*i)->start();

	showModal();

	m_status = 0;
	m_threads.resize(0);
	return true;
}

void BackgroundWorkerDialog::eventAbortClick(Event* event)
{
	m_buttonAbort->setEnable(false);
	
	// Stop all threads; assume timer polls until threads actually are finished.
	for (std::vector< Thread* >::const_iterator i = m_threads.begin(); i != m_threads.end(); ++i)
		(*i)->stop(0);
}

void BackgroundWorkerDialog::eventTimer(Event* event)
{
	// Check if job threads are finished.
	bool finished = true;
	for (std::vector< Thread* >::const_iterator i = m_threads.begin(); i != m_threads.end(); ++i)
	{
		if (!(*i)->finished())
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
		endModal(0);
}

		}
	}
}
